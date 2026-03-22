// g++ ncnn.cpp -o ncnn_app $(pkg-config --cflags --libs opencv4) -I/home/super/msv/ncnn/build/install/include -L/home/super/msv/ncnn/build/install/lib -lncnn -lglslang -lMachineIndependent -lGenericCodeGen  -lOSDependent -lSPIRV -lpthread -fopenmp
// !pip install ultralytics
// from ultralytics import YOLO
// model = YOLO("yolo26n.pt")
// model.export(format="ncnn", imgsz=320, half=False, end2end=False)  # ensure non-end2end if you want raw output + custom NMS


#include <opencv2/opencv.hpp>
#include <ncnn/net.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>

#define CONFIDENCE_THRESHOLD 0.75f
#define TARGET_SIZE 320

std::vector<std::string> classes = {
    "person","bicycle","car","motorcycle","airplane","bus","train","truck","boat","traffic light",
    "fire hydrant","stop sign","parking meter","bench","bird","cat","dog","horse","sheep","cow",
    "elephant","bear","zebra","giraffe","backpack","umbrella","handbag","tie","suitcase","frisbee",
    "skis","snowboard","sports ball","kite","baseball bat","baseball glove","skateboard","surfboard",
    "tennis racket","bottle","wine glass","cup","fork","knife","spoon","bowl","banana","apple",
    "sandwich","orange","broccoli","carrot","hot dog","pizza","donut","cake","chair","couch",
    "potted plant","bed","dining table","toilet","tv","laptop","mouse","remote","keyboard","cell phone",
    "microwave","oven","toaster","sink","refrigerator","book","clock","vase","scissors","teddy bear",
    "hair drier","toothbrush"
};

void RunObjectDetection()
{
    ncnn::Net net;

    // Critical: disable all FP16 paths for YOLO26
    net.opt.use_vulkan_compute = true;
    net.opt.use_fp16_arithmetic = false;
    net.opt.use_fp16_packed = false;
    net.opt.use_fp16_storage = false;

    net.opt.lightmode = true;
net.opt.num_threads = 4;   // or 8 depending on your CPU

net.opt.use_winograd_convolution = true;
net.opt.use_sgemm_convolution = true;
net.opt.use_packing_layout = true;

// net.opt.use_shader_pack8 = false;  // safer for transformer models
// net.opt.use_image_storage = false; // avoid slow paths


    if (net.load_param("yolo26_end_to_end/model.ncnn.param"))
    {
        std::cerr << "Failed to load param\n";
        return;
    }
    if (net.load_model("yolo26_end_to_end/model.ncnn.bin"))
    {
        std::cerr << "Failed to load model\n";
        return;
    }

    cv::VideoCapture cap(0, cv::CAP_V4L2);// Force V4L2 backend 

// Also, explicitly set the format to MJPG to save CPU on the Zero 2W
cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
cap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
cap.set(cv::CAP_PROP_FRAME_HEIGHT, 320);
// cap.set(cv::CAP_PROP_FPS, 5); // Don't ask for 30fps if we only want 2!

    if (!cap.isOpened())
    {
        std::cerr << "Camera error\n";
        return;
    }

    bool printed_shape = false;

    while (true)
    {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty())
            continue;

        int img_w = frame.cols;
        int img_h = frame.rows;

        auto t0 = std::chrono::high_resolution_clock::now();

        // Letterbox to TARGET_SIZE x TARGET_SIZE
        float scale = std::min((float)TARGET_SIZE / img_w, (float)TARGET_SIZE / img_h);
        int w = (int)(img_w * scale);
        int h = (int)(img_h * scale);

        ncnn::Mat in = ncnn::Mat::from_pixels_resize(
            frame.data, ncnn::Mat::PIXEL_BGR2RGB,
            img_w, img_h, w, h);

        int wpad = (TARGET_SIZE - w) / 2;
        int hpad = (TARGET_SIZE - h) / 2;

        ncnn::Mat in_pad;
        ncnn::copy_make_border(
            in, in_pad,
            hpad, TARGET_SIZE - h - hpad,
            wpad, TARGET_SIZE - w - wpad,
            ncnn::BORDER_CONSTANT, 114.f);

        const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
        in_pad.substract_mean_normalize(0, norm_vals);

        // Inference
        ncnn::Extractor ex = net.create_extractor();
        ex.input("in0", in_pad);

        ncnn::Mat out;
        ex.extract("out0", out);

        if (!printed_shape)
        {
            std::cout << "out.w=" << out.w
                      << " out.h=" << out.h
                      << " out.c=" << out.c << std::endl;
            printed_shape = true;
        }

        if (!(out.w == 2100 && out.h == 84 && out.c == 1))
        {
            std::cerr << "Unexpected out shape\n";
            cv::imshow("YOLO26 NCNN", frame);
            if (cv::waitKey(1) == 27) break;
            continue;
        }

        std::vector<cv::Rect> boxes;
        std::vector<int> class_ids;
        std::vector<float> scores;

        const int num_preds = 2100;

        for (int i = 0; i < num_preds; i++)
        {
            // Layout: attribute j is row(j), prediction index is column i
            float cx = out.row(0)[i];
            float cy = out.row(1)[i];
            float bw = out.row(2)[i];
            float bh = out.row(3)[i];

            float best_score = 0.f;
            int best_class = -1;

            // Classes are already sigmoid-ed in the graph (sigmoid_0)
            for (int j = 0; j < 80; j++)
            {
                float s = out.row(4 + j)[i];
                if (s > best_score)
                {
                    best_score = s;
                    best_class = j;
                }
            }

            if (best_score < CONFIDENCE_THRESHOLD ||
                best_class < 0 || best_class >= (int)classes.size())
                continue;

            // Heuristic: if bw/bh are small, treat as normalized [0,1]
            bool normalized = (bw <= 2.f && bh <= 2.f);

            if (normalized)
            {
                cx *= TARGET_SIZE;
                cy *= TARGET_SIZE;
                bw *= TARGET_SIZE;
                bh *= TARGET_SIZE;
            }

            float x1 = cx - bw * 0.5f;
            float y1 = cy - bh * 0.5f;
            float x2 = cx + bw * 0.5f;
            float y2 = cy + bh * 0.5f;

            // Clip to model space
            x1 = std::max(0.f, std::min((float)TARGET_SIZE, x1));
            y1 = std::max(0.f, std::min((float)TARGET_SIZE, y1));
            x2 = std::max(0.f, std::min((float)TARGET_SIZE, x2));
            y2 = std::max(0.f, std::min((float)TARGET_SIZE, y2));

            // Undo letterbox
            x1 = (x1 - wpad) / scale;
            y1 = (y1 - hpad) / scale;
            x2 = (x2 - wpad) / scale;
            y2 = (y2 - hpad) / scale;

            int ix1 = std::max(0, std::min(img_w - 1, (int)std::round(x1)));
            int iy1 = std::max(0, std::min(img_h - 1, (int)std::round(y1)));
            int ix2 = std::max(0, std::min(img_w - 1, (int)std::round(x2)));
            int iy2 = std::max(0, std::min(img_h - 1, (int)std::round(y2)));

            int bw_i = ix2 - ix1;
            int bh_i = iy2 - iy1;
            if (bw_i <= 0 || bh_i <= 0)
                continue;

            boxes.emplace_back(ix1, iy1, bw_i, bh_i);
            class_ids.push_back(best_class);
            scores.push_back(best_score);
        }

        // Draw detections (no NMS for now)
        for (size_t i = 0; i < boxes.size(); i++)
        {
            cv::rectangle(frame, boxes[i], cv::Scalar(0, 255, 0), 2);
            std::string label = cv::format("%s %.2f",
                                           classes[class_ids[i]].c_str(),
                                           scores[i]);
            cv::putText(frame, label,
                        cv::Point(boxes[i].x, boxes[i].y - 5),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5,
                        cv::Scalar(0, 255, 0), 1);
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        if (ms > 0)
        {
            cv::putText(frame, cv::format("%.1f FPS", 1000.0 / ms),
                        cv::Point(10, 30),
                        cv::FONT_HERSHEY_SIMPLEX, 0.8,
                        cv::Scalar(0, 0, 255), 2);
        }

        cv::imshow("YOLO26 NCNN", frame);
        if (cv::waitKey(1) == 27)
            break;
    }
}

int main()
{
    RunObjectDetection();
    return 0;
}

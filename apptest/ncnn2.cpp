// g++ ncnn2.cpp -o ncnn2 $(pkg-config --cflags --libs opencv4) -I/home/super/msv/ncnn/build/install/include/ncnn -L/home/super/msv/ncnn/build/install/lib -lncnn -lglslang -lMachineIndependent -lGenericCodeGen  -lOSDependent -lSPIRV -lpthread -fopenmp

#include "net.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main() {
    ncnn::Net yolo;
    yolo.opt.use_vulkan_compute = false; 
    yolo.opt.num_threads = 4; 

    // 1. Load Model
    if (yolo.load_param("model.ncnn.param") || yolo.load_model("model.ncnn.bin")) {
        std::cerr << "Model load failed!" << std::endl;
        return -1;
    }

    // 2. Open Camera with V4L2 (No GStreamer bloat)
    cv::VideoCapture cap(0, cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 320);

    if (!cap.isOpened()) {
        std::cerr << "Camera open failed!" << std::endl;
        return -1;
    }

    std::cout << "Starting Headless Inference... Press Ctrl+C to stop." << std::endl;

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        // Preprocess
        ncnn::Mat in = ncnn::Mat::from_pixels_resize(frame.data, 
            ncnn::Mat::PIXEL_BGR2RGB, frame.cols, frame.rows, 320, 320);

        const float mean_vals[3] = {0.f, 0.f, 0.f};
        const float norm_vals[3] = {1/255.f, 1/255.f, 1/255.f};
        in.substract_mean_normalize(mean_vals, norm_vals);

        // 3. Inference inside a scope to free RAM immediately
        {
            ncnn::Extractor ex = yolo.create_extractor();
            ex.input("in0", in); 
            
            ncnn::Mat out;
            ex.extract("out0", out); 

            // Post-process: Print to terminal instead of drawing
            for (int i = 0; i < out.h; i++) {
                const float* values = out.row(i);
                float score = values[4];

                if (score > 0.45f) { // Higher threshold for terminal output
                    int class_id = (int)values[5];
                    std::cout << "[DETECT] ID: " << class_id 
                              << " Conf: " << score 
                              << " at [" << values[0] << "," << values[1] << "]" << std::endl;
                }
            }
        } // <--- 'ex' and 'out' are destroyed here, clearing workspace RAM

        // No cv::imshow() or cv::waitKey() here!
        cv::imshow("YOLO26 Pi Zero 2W", frame);
        if (cv::waitKey(1) == 27) break; // ESC to exit
    }

    return 0;
}
#include <opencv2/opencv.hpp>
#include <openvino/openvino.hpp>
#include <iostream>
#include <vector> 
#include <chrono> 
#include "general.hpp"

#define CONFIDENCE_THRESHOLD 0.2
#define NMS_THRESHOLD 0.45


std::vector<std::string> classes = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};

void RunObjectDetection() {
    ov::Core core;
    auto model = core.read_model("yolov10n_fp32_openvino/yolov10n.xml");
    // auto model = core.read_model("yolov10n_int8_openvino/yolov10n.xml");
    auto compiled_model = core.compile_model(model, "CPU");
    auto infer_request = compiled_model.create_infer_request(); 

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Erro ao abrir a câmera!" << std::endl;
        return;
    }
// getchar();
    while (true) {
        cv::Mat frame, resized, padded, blob;
        cap >> frame;
        if (frame.empty()) continue; 
        cv::flip(frame,frame,1);
        auto clkstart = std::chrono::high_resolution_clock::now();

        float scale = std::min(640.0f / frame.cols, 640.0f / frame.rows);
        cv::resize(frame, resized, cv::Size(), scale, scale);
        int pad_w = 640 - resized.cols, pad_h = 640 - resized.rows;
        cv::copyMakeBorder(resized, padded, 0, pad_h, 0, pad_w, cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));
        cv::dnn::blobFromImage(padded, blob, 1.0 / 255.0, cv::Size(), cv::Scalar(), true);
        // cv::dnn::blobFromImage(frame, blob, 1.0 / 255.0, cv::Size(), cv::Scalar(), true);
        
        // cotm(scale,padded.rows,padded.cols,frame.rows,frame.cols);
        // cv::imshow("YOLO Inference", padded);
        // getchar();
        // continue;


        ov::Tensor input_tensor(ov::element::f32, {1, 3, 640, 640}, blob.ptr<float>());
        infer_request.set_input_tensor(input_tensor);
        infer_request.infer();

        const float* output_data = infer_request.get_output_tensor(0).data<const float>();
        const ov::Shape output_shape = infer_request.get_output_tensor(0).get_shape();
        int num_boxes = output_shape[2];

        std::vector<cv::Rect> boxes;
        std::vector<float> confidences;
        std::vector<int> class_ids;
        for (int i = 0; i < num_boxes; i++) {
            const float* entry = output_data + i * 84;
            float confidence = entry[4];
            if (confidence < CONFIDENCE_THRESHOLD) continue;
            int x=entry[0],y=entry[1],w=entry[2]-entry[0],h=entry[3];
            boxes.emplace_back(x, y, w, h);
            confidences.push_back(confidence); 
            class_ids.push_back((int)entry[5]);
        }
        

        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);

        for (int idx : indices) { 
            cv::Rect box = boxes[idx];
            std::string label = cv::format("%s %.2f", classes[class_ids[idx]].c_str(), confidences[idx]);
            
            cv::rectangle(frame, box, cv::Scalar(0, 255, 0), 2);
            cv::putText(frame, label, box.tl() + cv::Point(0, -10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
        }
        // Display inference speed
        
        auto clkend = std::chrono::high_resolution_clock::now();
        double inference_time = std::chrono::duration_cast<std::chrono::milliseconds>(clkend - clkstart).count();
        cv::putText(frame, cv::format("inference: %.0f ms",  inference_time), {10, 25}, cv::FONT_HERSHEY_SIMPLEX, 0.7, {0, 0, 0}, 2);

        cv::imshow("YOLO Inference", frame);
        if (cv::waitKey(1) == 27) break;
    }
    cap.release();
    cv::destroyAllWindows();
}

int main() {
    RunObjectDetection();
    return 0;
}


#include "ObjectDetector.hpp"

ObjectDetector::ObjectDetector(std::string modelConfigurationPath, std::string modelWeightsPath, std::string classesFilePath) {
    // Load the neural network
    net = readNetFromDarknet(modelConfigurationPath, modelWeightsPath);
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);

    // Load class names
    classNames = getClassNames(classesFilePath);
}

// Function to get class names
std::vector<std::string> ObjectDetector::getClassNames(const std::string &classFilePath) {
    std::vector<std::string> classes;
    std::ifstream ifs(classFilePath.c_str());
    std::string line;
    while (getline(ifs, line)) {
        classes.emplace_back(line);
    }
    return classes;
}

// Function to get YOLO output layer names
const std::vector<String> &ObjectDetector::getOutputsNames(const Net &net) {
    static std::vector<String> names;
    if (names.empty()) {
        // Get indices of output layers
        const std::vector<int> outLayers = net.getUnconnectedOutLayers();
        // Get names of all layers in the network
        const std::vector<String> layersNames = net.getLayerNames();
        // Get the names of the output layers using their indices
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i) {
            names[i] = layersNames[outLayers[i] - 1];
        }
    }
    return names;
}

std::string ObjectDetector::formatFloat(float value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

// Function to draw bounding boxes
void ObjectDetector::drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat &frame,
              const std::vector<std::string> &classNames) {
    rectangle(frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);

    std::string label = formatFloat(conf);
    if (!classNames.empty()) {
        CV_Assert(classId < classNames.size());
        label = classNames[classId] + ": " + label;
    }

    int baseLine;
    const auto labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    top = max(top, labelSize.height);
    rectangle(frame, Point(left, top - labelSize.height),
              Point(left + labelSize.width, top + baseLine), Scalar::all(255), FILLED);
    putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 1);
}

// Function to detect objects in an image
Mat ObjectDetector::detectObjects(Mat frame, std::vector<int>& coords, const std::string &objectName) {
    // Read the image
    Mat blob;
    Size size(416, 416);

    blobFromImage(frame, blob, 1/255.0, size, {}, true, false);
    net.setInput(blob);

    // Run forward pass
    std::vector<Mat> outs;
    net.forward(outs, getOutputsNames(net));

    // Post-process the output
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<Rect> boxes;
    float confThreshold = 0.5f;
    float nmsThreshold = 0.4f;

    for (const auto &out: outs) {
        auto data = reinterpret_cast<float *>(out.data);
        for (int j = 0; j < out.rows; ++j, data += out.cols) {
            Mat scores = out.row(j).colRange(5, out.cols);
            Point classIdPoint;
            double confidence;
            minMaxLoc(scores, nullptr, &confidence, nullptr, &classIdPoint);
            if (confidence > confThreshold) {
                int centerX = static_cast<int>(data[0] * frame.cols);
                int centerY = static_cast<int>(data[1] * frame.rows);
                int width = static_cast<int>(data[2] * frame.cols);
                int height = static_cast<int>(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;
                classIds.push_back(classIdPoint.x);
                confidences.emplace_back(static_cast<float>(confidence));
                boxes.emplace_back(left, top, width, height);
            }
        }

    }

    // Non-maximum suppression to remove redundant overlapping boxes
    std::vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    for (int idx: indices) {
        Rect box = boxes[idx];
        // push coordinates of the center of the object to the vector
        if (classNames[classIds[idx]] == objectName && coords.size() < 2) {
            coords.push_back(box.x + box.width / 2);
            coords.push_back(box.y + box.height / 2);
        }
        // draw bounding box
        drawPred(classIds[idx], confidences[idx], box.x, box.y, box.x + box.width, box.y + box.height, frame,
                 classNames);
    }

    return frame;
}

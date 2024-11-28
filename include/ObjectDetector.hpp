
#ifndef RVR_SERVER_OBJECTDETECTOR_HPP
#define RVR_SERVER_OBJECTDETECTOR_HPP

#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <fstream>
#include <vector>

using namespace cv;
using namespace dnn;

class ObjectDetector {
private:
    std::vector<std::string> classNames;
    Net net;

    std::vector<std::string> getClassNames(const std::string &classFilePath);

    const std::vector<std::string> &getOutputsNames(const Net& net);

    void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat &frame,const std::vector<std::string> &classNames);

    std::string formatFloat(float value);
public:
    ObjectDetector(std::string modelConfigurationPath, std::string modelWeightsPath, std::string classesFilePath);
    Mat detectObjects(Mat frame, std::vector<int>& coords, const std::string &objectName);
};

#endif //RVR_SERVER_OBJECTDETECTOR_HPP

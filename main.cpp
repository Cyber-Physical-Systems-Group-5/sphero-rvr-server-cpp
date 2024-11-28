#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>
#include "include/CommunicationHandler.hpp"
#include "src/util/Message.hpp"
#include "include/KeyListener.hpp"
#include "ObjectDetector.hpp"

int main() {
    CommunicationHandler server(8000);
    KeyListener keyListener;
    std::string yoloConfigPath = "/media/kirk/TOSHIBA/dev/project/sphero-rvr-server-cpp/data/yolov7-tiny.cfg";
    std::string yoloWeightsPath = "/media/kirk/TOSHIBA/dev/project/sphero-rvr-server-cpp/data/yolov7-tiny.weights";
    std::string yoloClassesPath = "/media/kirk/TOSHIBA/dev/project/sphero-rvr-server-cpp/data/coco.names";
    ObjectDetector objectDetector(yoloConfigPath, yoloWeightsPath, yoloClassesPath);
    std::atomic<bool> isRunning{true};
    std::cout << "Server started on port 8000" << std::endl;
    // start thread to listen for key presses and send commands
    std::jthread keyListenerThread([&keyListener, &server, &isRunning] {
        while (isRunning) {
            {
                std::unique_lock<std::mutex> lock(keyListener.getMtx());
                keyListener.cv.wait(lock, [&] { return keyListener.hasMessages(); });
            }
            while (keyListener.hasMessages()) {
                auto keyMessage = keyListener.getMessage();
                server.write(keyMessage);
            }
        }
    });

    // Variables for FPS calculation
    auto lastTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    while (isRunning) {

        {
            std::unique_lock<std::mutex> lock(server.getMtx());
            server.cv.wait(lock, [&] { return server.hasMessages(); });
        }

        // Retrieve and process messages
        while (server.hasMessages()) {
            Message message = server.getLatestMessage();

            // Process image if available
            if (message.getImage().has_value()) {
                auto receivedImage = message.getImage().value();
                std::vector<unsigned char> imageBytes(receivedImage.begin(), receivedImage.end());
                std::vector<int> coords;

                cv::Mat image = cv::imdecode(imageBytes, cv::IMREAD_COLOR);
                cv::Mat frame = objectDetector.detectObjects(image, coords, "bottle");
                cv::imshow("Received Image", frame);
                cv::waitKey(1);
            }

            // FPS calculation
            frameCount++;
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime);

            if (duration.count() >= 1) {
                std::cout << "FPS: " << frameCount << std::endl;
                frameCount = 0;
                lastTime = currentTime;
            }
        }
    }
}

#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>
#include "include/CommunicationHandler.hpp"
#include "src/util/Message.hpp"

int main() {
    uint16_t TCPPort = 8000;
    uint16_t UDPPort = 8001;
    std::string remoteAddress = "10.22.88.158";
    CommunicationHandler server(TCPPort, UDPPort, remoteAddress, UDPPort);
    std::cout << "Server started on port 8000" << std::endl;
    std::cout << "Press 'q' to quit" << std::endl;
    while (true) {
        if (server.hasMessages()) {
            Message message = server.getLatestImageMessage();
            std::cout << "Speed: " << static_cast<int>(message.getSpeed()) << std::endl;
            std::cout << "Directions: ";
            for (const auto &direction : message.getDirections()) {
                std::cout << static_cast<int>(direction) << " ";
            }
            std::cout << std::endl;
            if (message.getImage().has_value()) {
                auto receivedImage = message.getImage().value();
                std::cout << "Image size: " << receivedImage.size() << std::endl;
                std::vector<unsigned char> imageBytes(receivedImage.begin(), receivedImage.end());

                cv::Mat image = cv::imdecode(imageBytes, cv::IMREAD_COLOR);
                cv::imshow("Received Image", image);
                cv::waitKey(0);
            }
        }
    }
}

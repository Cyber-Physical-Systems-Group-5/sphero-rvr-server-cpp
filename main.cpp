#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>
#include "include/CommunicationHandler.hpp"
#include "src/util/Message.hpp"
#include "include/KeyListener.hpp"

int main() {
    CommunicationHandler server(8000);
    KeyListener keyListener;
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

    while (isRunning) {

        {
            std::unique_lock<std::mutex> lock(server.getMtx());
            server.cv.wait(lock, [&] { return server.hasMessages(); });
        }

        // Retrieve and process messages
        while (server.hasMessages()) {
            Message message = server.getLatestMessage();

            // Process directions
            for (const auto& direction : message.getDirections()) {
                std::cout << static_cast<int>(direction) << " ";
            }
            std::cout << std::endl;

            // Process image if available
            if (message.getImage().has_value()) {
                auto receivedImage = message.getImage().value();
                std::vector<unsigned char> imageBytes(receivedImage.begin(), receivedImage.end());

                cv::Mat image = cv::imdecode(imageBytes, cv::IMREAD_COLOR);
                cv::imshow("Received Image", image);
                cv::waitKey(1);
            }
        }
    }
}

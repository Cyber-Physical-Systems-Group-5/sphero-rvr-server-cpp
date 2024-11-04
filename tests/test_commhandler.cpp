#include "CommunicationHandler.hpp"
#include "Message.hpp"
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <fstream>
#include <condition_variable>
#include <filesystem>
#include <chrono>
#include <iostream>

std::string loadImage(std::filesystem::path path) {

    std::ifstream fileStream(path);
    if (!fileStream) {
        throw std::runtime_error("Failed to open file at: " + path.string());
    }

    std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    return fileContent;
}

TEST_CASE("CommunicationHandler read/write") {
    uint16_t port = 8000;
    std::condition_variable cv;
    std::mutex cvMutex;
    bool serverReady = false;

    std::string image = loadImage(IMAGE_PATH);

    Message message1 = Message::fromJSONString("{\"speed\": 100, \"directions\": [\"forward\", \"left\"]}");
    Message message2 = Message::fromJSONString("{\"speed\": 50, \"directions\": [\"backward\", \"right\"]}");
    message2.setImageFromString(image);

    // Store message sizes
    const uint32_t message1Size = message1.toProto().size();
    const uint32_t message2Size = message2.toProto().size();




    std::thread serverThread([&] {
        CommunicationHandler server(port);
        // Signal that the server is ready
        {
            std::lock_guard<std::mutex> lock(cvMutex);
            serverReady = true;
        }
        cv.notify_one();

        // Wait 50 ms for client to connect and send the data
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Wait for and read client message1
        auto response = server.getLatestMessage();
        CHECK(response == message1);
        response = server.getLatestMessage();
        CHECK(response == message2);

        // test write
        //server.write(message1);
    });

    // Wait for server to be ready
    {
        std::unique_lock<std::mutex> lock(cvMutex);
        cv.wait(lock, [&serverReady] { return serverReady; });
    }

    // Start client thread only after server is ready
    std::thread clientThread([&] {
        TCPClientContext client;
        const auto conn = client.connect("127.0.0.1", port);
        REQUIRE(conn);
        // send first message
        conn->write(reinterpret_cast<const unsigned char *>(&message1Size), sizeof(uint32_t));
        conn->write(message1.toProto());
        // send second message
        conn->write(reinterpret_cast<const unsigned char *>(&message2Size), sizeof(uint32_t));
        conn->write(message2.toProto());

    });

    clientThread.join();
    serverThread.join();
}

TEST_CASE("CommunicationHandler message processing rate") {
    uint16_t port = 8000;
    std::condition_variable cv;
    std::mutex cvMutex;
    bool serverReady = false;

    // Load and prepare a sample image
    std::string image = loadImage(IMAGE_PATH);

    // Prepare a message with an image
    Message imageMessage = Message::fromJSONString("{\"speed\": 100, \"directions\": [\"forward\", \"left\"]}");
    imageMessage.setImageFromString(image);

    // Store message size
    const uint32_t messageSize = imageMessage.toProto().size();

    const int messageCount = 300; // Define the number of messages to send for the test
    int receivedCount = 0;

    std::thread serverThread([&] {
        CommunicationHandler server(port);

        // Signal that the server is ready
        {
            std::lock_guard<std::mutex> lock(cvMutex);
            serverReady = true;
        }
        cv.notify_one();

        // Start receiving and counting messages
        auto startTime = std::chrono::steady_clock::now();
        while (receivedCount < messageCount) {
            if (server.getLatestMessage() != Message()) {
                receivedCount++;
            }
        }
        auto endTime = std::chrono::steady_clock::now();

        // Calculate messages per second
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        double messagesPerSecond = static_cast<double>(messageCount) / (duration / 1000.0);

        CHECK(messagesPerSecond > 30);

        std::cout << "Processed " << messagesPerSecond << " messages per second.\n";
    });

    // Wait for server to be ready
    {
        std::unique_lock<std::mutex> lock(cvMutex);
        cv.wait(lock, [&serverReady] { return serverReady; });
    }

    // Start client thread after server is ready
    std::thread clientThread([&] {
        TCPClientContext client;
        const auto conn = client.connect("127.0.0.1", port);
        REQUIRE(conn);

        for (int i = 0; i < messageCount; ++i) {
            conn->write(reinterpret_cast<const unsigned char *>(&messageSize), sizeof(uint32_t));
            conn->write(imageMessage.toProto());
        }
    });

    clientThread.join();
    serverThread.join();
}
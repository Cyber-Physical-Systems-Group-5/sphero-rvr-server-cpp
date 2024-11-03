#include "CommunicationHandler.hpp"
#include "Message.hpp"
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <fstream>
#include <condition_variable>
#include <filesystem>

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

    Message message1 = Message::fromString("{\"speed\": 100, \"directions\": [\"forward\", \"left\"]}");
    Message message2 = Message::fromString("{\"speed\": 50, \"directions\": [\"backward\", \"right\"]}");
    message2.setImageFromString(image);




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
        server.write(message1);
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
        conn->write(message1.toString() + "|");
        conn->write(message2.toString() + "|");

    });

    clientThread.join();
    serverThread.join();
}
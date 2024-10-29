#include "CommunicationHandler.hpp"
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "image.pb.h"

std::string loadImage(std::filesystem::path path) {

    std::ifstream fileStream(path);
    if (!fileStream) {
        throw std::runtime_error("Failed to open file at: " + path.string());
    }

    std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    return fileContent;
}

TEST_CASE("CommunicationHandler read/write") {
    std::string domain = "test";


    std::thread serverThread([&] {
        CommunicationHandler server(domain);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto response = server.read();
        REQUIRE(response == "test_message");

    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::thread clientThread([&] {

        UnixDomainClientContext client;
        const auto conn = client.connect(domain);
        REQUIRE(conn);

        std::string message = "test_message";
        conn->write(message);
    });

    clientThread.join();
    serverThread.join();
}

TEST_CASE("CommunicationHandler protoc read/write") {
    std::string domain = "test";
    std::filesystem::path imagePath = std::filesystem::path(PROJECT_SOURCE_DIR) / "tests/data/lenna.png";
    const auto data = loadImage(imagePath);

    // print hash of original image
    std::hash<std::string> hasher;
    std::string original_hash = std::to_string(hasher(data));
    //std::cout << "Original image hash: " << original_hash << std::endl;


    std::thread serverThread([&] {
        CommunicationHandler server(domain);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::string serializedMessage = server.read();
        test::Image receivedImage;

        // print received image before deserialization hash
        //std::cout << "Received image hash before deserialization: " << std::to_string(hasher(serializedMessage)) << std::endl;
        REQUIRE(receivedImage.ParseFromString(serializedMessage));
        //std::cout << "Received image hash after deserialization: " << std::to_string(hasher(receivedImage.data())) << std::endl;
        REQUIRE(receivedImage.data() == data);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::thread clientThread([&] {
        test::Image image;
        image.set_data(data);

        // Serialize the image
        std::string serialized;
        image.SerializeToString(&serialized);

        // print serialized string hash
        //std::cout << "Serialized string hash: " << std::to_string(hasher(serialized)) << std::endl;

        UnixDomainClientContext client;
        const auto conn = client.connect(domain);
        REQUIRE(conn);

        //std::cout << "Serialized string" << serialized << std::endl;

        conn->write(serialized);
    });

    clientThread.join();
    serverThread.join();
}


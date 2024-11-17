#include "CommunicationHandler.hpp"
#include "Message.hpp"
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <fstream>
#include <condition_variable>
#include <filesystem>
#include <chrono>
#include <iostream>

#define HEADER_SIZE sizeof(uint32_t)

std::string loadImage(std::filesystem::path path) {

    std::ifstream fileStream(path);
    if (!fileStream) {
        throw std::runtime_error("Failed to open file at: " + path.string());
    }

    std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    return fileContent;
}

std::vector<std::vector<unsigned char>> prepareUDPPackets(const Message& message) {
    std::vector<std::vector<unsigned char>> packets;

    // Serialize the message to a string
    std::string protoString = message.toProto();
    const uint32_t messageSize = protoString.size();

    // Calculate the payload size for the first packet
    const uint32_t payloadSize = MAX_UDP_PACKET_SIZE - HEADER_SIZE;

    // Calculate the total number of packets needed
    uint32_t totalPackets = (messageSize + payloadSize - 1) / payloadSize; // Ceiling of messageSize / payloadSize

    // The first packet will contain the total bytes of the message as header
    {
        uint32_t chunkSize = std::min(payloadSize, messageSize);

        // Create the first packet
        std::vector<unsigned char> firstPacket(HEADER_SIZE + chunkSize);

        // Add the header (message size as 4 bytes)
        std::memcpy(firstPacket.data(), &messageSize, HEADER_SIZE);

        // Add the payload
        std::memcpy(firstPacket.data() + HEADER_SIZE, protoString.data(), chunkSize);

        packets.push_back(std::move(firstPacket));
    }

    // Split the message into multiple packets
    for (uint32_t i = 1; i < totalPackets; ++i) {
        uint32_t startIdx = i * payloadSize;
        uint32_t endIdx = std::min(startIdx + payloadSize, messageSize);
        uint32_t chunkSize = endIdx - startIdx;

        // Create the packet
        std::vector<unsigned char> packet(chunkSize);

        // Add the payload
        std::memcpy(packet.data(), protoString.data() + startIdx, chunkSize);

        packets.push_back(std::move(packet));
    }

    return packets;
}


TEST_CASE("CommunicationHandler read/write TCP") {
    uint16_t TCPPort = 8000;
    uint16_t UDPPort = 8340;
    std::string remoteAddress = "127.0.0.1";
    std::condition_variable cv;
    std::mutex cvMutex;
    bool serverReady = false;


    Message message = Message::fromJSONString("{\"speed\": 100, \"directions\": [\"forward\", \"left\"], \"MessageType\": 1}"); // COMMAND type
    // Store message sizes
    const uint32_t messageSize = message.toProto().size();




    std::thread serverThread([&] {
        CommunicationHandler server(TCPPort, UDPPort, remoteAddress, UDPPort);
        // Signal that the server is ready
        {
            std::lock_guard<std::mutex> lock(cvMutex);
            serverReady = true;
        }
        cv.notify_one();

        // Wait 50 ms for client to connect and send the data
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Wait for and read client message1
        auto response = server.getLatestCommandMessage();
        CHECK(response == message);

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
        const auto conn = client.connect("127.0.0.1", TCPPort);
        REQUIRE(conn);
        // send first message
        conn->write(reinterpret_cast<const unsigned char *>(&messageSize), sizeof(uint32_t));
        conn->write(message.toProto());

    });

    clientThread.join();
    serverThread.join();
}

TEST_CASE("CommunicationHandler read/write UDP") {
    uint16_t TCPPort = 8000;
    uint16_t UDPServerPort = 8001;
    uint16_t UDPClientPort = 8002;
    std::string remoteAddress = "127.0.0.1";

    CommunicationHandler server(TCPPort, UDPServerPort, remoteAddress, UDPClientPort);

    auto image = loadImage(IMAGE_PATH);
    Message message = Message::fromJSONString("{\"speed\": 100, \"directions\": [\"forward\", \"left\"], \"MessageType\": 0}"); // IMAGE type
    message.setImageFromString(image);

    // Store message sizes
    auto packets = prepareUDPPackets(message);

    // send udp packets
    UDPSocket client(UDPClientPort);
    for (const auto &packet : packets) {
        REQUIRE(client.sendTo(remoteAddress, UDPServerPort, packet));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    Message response;

    while ((response = server.getLatestImageMessage()).getType() == Type::EMPTY) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    CHECK(response == message);


}

TEST_CASE("CommunicationHandler message processing rate") {
    uint16_t TCPPort = 8000;
    uint16_t UDPPort = 8001;
    std::string remoteAddress = "127.0.0.1";
    std::condition_variable cv;
    std::mutex cvMutex;
    bool serverReady = false;

    // Load and prepare a sample image
    std::string image = loadImage(IMAGE_PATH);

    // Prepare a message with an image
    Message imageMessage = Message::fromJSONString("{\"speed\": 100, \"directions\": [\"forward\", \"left\"], \"MessageType\": 0}"); // IMAGE type
    imageMessage.setImageFromString(image);

    // Store message size
    const uint32_t messageSize = imageMessage.toProto().size();

    const int messageCount = 300; // Define the number of messages to send for the test
    int receivedCount = 0;

    std::thread serverThread([&] {
        CommunicationHandler server(TCPPort, UDPPort, remoteAddress, UDPPort);;

        // Signal that the server is ready
        {
            std::lock_guard<std::mutex> lock(cvMutex);
            serverReady = true;
        }
        cv.notify_one();

        // Start receiving and counting messages
        auto startTime = std::chrono::steady_clock::now();
        while (receivedCount < messageCount) {
            if (server.getLatestImageMessage() != Message()) {
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
        const auto conn = client.connect("127.0.0.1", TCPPort);
        REQUIRE(conn);

        for (int i = 0; i < messageCount; ++i) {
            conn->write(reinterpret_cast<const unsigned char *>(&messageSize), sizeof(uint32_t));
            conn->write(imageMessage.toProto());
        }
    });

    clientThread.join();
    serverThread.join();
}
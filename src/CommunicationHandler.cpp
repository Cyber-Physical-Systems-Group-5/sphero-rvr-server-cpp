#include <string>
#include <fstream>
#include "../include/CommunicationHandler.hpp"

CommunicationHandler::CommunicationHandler(uint16_t port) : server(port, 1) {
    connectionThread = std::jthread(&CommunicationHandler::handleConnection, this);
}

void CommunicationHandler::close() {
    isRunning = false;
    if (connectionThread.joinable()) {
        connectionThread.join();
    }
    connection->close();
}

void CommunicationHandler::read() {
    if (!connection) {
        return;
    }

    std::vector<unsigned char> buffer(1024);
    int bytesRead = 0;

    // Store any remaining part of a message from previous reads
    static std::string leftover;

    // Store expected length of next message
    static size_t expectedMessageLength = 0;

    // Loop to accumulate data until we reach a complete message
    while ((bytesRead = connection->read(buffer)) > 0) {
        leftover.append(buffer.begin(), buffer.begin() + bytesRead);

        while (true) {
            if (expectedMessageLength == 0) {
                // If we haven't read the length yet, check if we have enough bytes for it
                if (leftover.size() < sizeof(uint32_t)) {
                    break; // wait for more data
                }

                // Extract message length (first 4 bytes)
                uint32_t lengthNetworkOrder;
                std::memcpy(&lengthNetworkOrder, leftover.data(), sizeof(uint32_t));
                expectedMessageLength = ntohl(lengthNetworkOrder);

                leftover.erase(0, sizeof(uint32_t)); // Remove the length prefix
            }

            // Check if we have the complete message
            if (leftover.size() < expectedMessageLength) {
                break; // wait for more data
            }

            // Extract the complete message based on the expected length
            std::string completeMessage = leftover.substr(0, expectedMessageLength);
            leftover.erase(0, expectedMessageLength);
            expectedMessageLength = 0; // Reset for next message

            // Process complete message
            Message receivedMessage = Message::fromProto(completeMessage);
            messageQueue.push(receivedMessage);
        }
    }
}

void CommunicationHandler::write(const Message& message) {
    if (!connection) {
        return;
    }
    // convert the message to a string
    auto messageString = message.toProto();
    uint32_t messageLength = messageString.size();

    // send the message
    connection->write(reinterpret_cast<const unsigned char *>(&messageLength), sizeof(uint32_t));
    connection->write(reinterpret_cast<const unsigned char *>(messageString.data()), messageString.size());
}

void CommunicationHandler::handleConnection() {
    connection = server.accept();
    if (!connection) {
        throw std::runtime_error("Failed to accept connection");
    }
    connectionCount++;

    while (isRunning) {
        read();
        // check if host is still connected
        if (!connection) {
            connectionCount--;
            handleConnection();
        }
    }
}

Message CommunicationHandler::getLatestMessage() {
    if (messageQueue.empty()) {
        return {};
    }

    Message message = messageQueue.front();
    messageQueue.pop();
    return message;
}

bool CommunicationHandler::hasMessages() const {
    return !messageQueue.empty();
}

uint32_t CommunicationHandler::ntohl(uint32_t net) {
    return ((net & 0xFF000000) >> 24) |
           ((net & 0x00FF0000) >> 8) |
           ((net & 0x0000FF00) << 8) |
           ((net & 0x000000FF) << 24);
}

CommunicationHandler::~CommunicationHandler() {
    close();
}

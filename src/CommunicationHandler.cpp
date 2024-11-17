#include <string>
#include <fstream>
#include <utility>
#include "../include/CommunicationHandler.hpp"
#include "Message.hpp"

CommunicationHandler::CommunicationHandler(uint16_t TCPPort, uint16_t UDPPort, std::string remoteAddress, uint16_t remotePort) :
server(TCPPort, 1),
udpSocket(UDPPort),
remoteAddress(std::move(remoteAddress)),
remotePort(remotePort) {
    TCPconnectionThread = std::jthread(&CommunicationHandler::handleTCPConnection, this);
    UDPconnectionThread = std::jthread(&CommunicationHandler::handleUDPConnection, this);

}

void CommunicationHandler::close() {
    isRunning = false;
    udpSocket.close();
    server.close();
    if (TCPconnectionThread.joinable() && UDPconnectionThread.joinable()) {
        TCPconnectionThread.join();
        UDPconnectionThread.join();
    }
    TCPconnection->close();
}

void CommunicationHandler::readTCP() {
    std::vector<unsigned char> buffer(1024);
    int bytesRead = 0;

    // Store any remaining part of a message from previous reads
    static std::string leftover;

    // Store expected length of next message
    static size_t expectedMessageLength = 0;

    // Loop to accumulate data until we reach a complete message
    while ((bytesRead = TCPconnection->read(buffer)) > 0) {
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
                expectedMessageLength = lengthNetworkOrder;

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
            commandQueue.push(receivedMessage);
        }
    }
}

void CommunicationHandler::readUDP() {
    std::vector<unsigned char> buffer(65536);
    int bytesRead = 0;

    // Store any remaining part of a message from previous reads
    static std::string leftover;

    // Store expected length of next message
    static size_t expectedMessageLength = 0;

    // Loop to accumulate data until we reach a complete message
    while ((bytesRead = udpSocket.recvFrom(remoteAddress, remotePort, buffer)) > 0) {
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
                expectedMessageLength = lengthNetworkOrder;

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
            Message receivedMessage;

            // discard message if corrupted
            try {
                receivedMessage = Message::fromProto(completeMessage);
            } catch (const std::exception& e) {
                continue;
            }

            // Check if the message is an image
            switch (receivedMessage.getType()) {
                case Type::IMAGE:
                    imageQueue.push(receivedMessage);
                    break;
                case Type::COMMAND:
                    commandQueue.push(receivedMessage);
                    break;
                default:
                    break;
            }
        }
    }
}

void CommunicationHandler::write(const Message& message) {
    if (!TCPconnection) {
        return;
    }
    // convert the message to a string
    auto messageString = message.toProto();
    uint32_t messageLength = messageString.size();

    // send the message
    TCPconnection->write(reinterpret_cast<const unsigned char *>(&messageLength), sizeof(uint32_t));
    TCPconnection->write(reinterpret_cast<const unsigned char *>(messageString.data()), messageString.size());
}

void CommunicationHandler::handleTCPConnection() {
    TCPconnection = server.accept();
    if (!TCPconnection) {
        throw std::runtime_error("Failed to accept TCPconnection");
    }
    connectionCount++;

    while (isRunning) {
        readTCP();
        // check if host is still connected
        if (!TCPconnection) {
            connectionCount--;
            handleTCPConnection();
        }
    }
}

void CommunicationHandler::handleUDPConnection() {
    while (isRunning) {
        readUDP();
    }
}

Message CommunicationHandler::getLatestCommandMessage() {
    if (commandQueue.empty()) {
        Message message;
        message.setType(Type::EMPTY);
        return message;
    }

    Message message = commandQueue.front();
    commandQueue.pop();
    return message;
}

Message CommunicationHandler::getLatestImageMessage() {
    if (imageQueue.empty()) {
        Message message;
        message.setType(Type::EMPTY);
        return message;
    }

    Message message = imageQueue.front();
    imageQueue.pop();
    return message;
}

bool CommunicationHandler::hasMessages() const {
    return !commandQueue.empty();
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

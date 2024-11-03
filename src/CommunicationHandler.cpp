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
    std::string response;
    int bytesRead = 0;

    // Store any remaining part of a message from previous reads
    static std::string leftover;

    // Loop to accumulate data until we reach a terminator
    while ((bytesRead = connection->read(buffer)) > 0) {
        response.append(buffer.begin(), buffer.begin() + bytesRead);

        // Look for the terminator character in the response
        size_t pos = 0;
        while ((pos = response.find('|')) != std::string::npos) {
            std::string completeMessage = leftover + response.substr(0, pos);
            leftover.clear();
            response.erase(0, pos + 1); // Remove processed part from response

            // Process complete Message
            Message receivedMessage = Message::fromString(completeMessage);
            messageQueue.push(receivedMessage);
        }

        // If no terminator found, add data to leftover and wait for the next chunk
        leftover += response;
        response.clear();
    }
}

void CommunicationHandler::write(const Message& message) {
    if (!connection) {
        return;
    }
    // convert the message to a string
    auto messageString = message.toString() + "|";
    // send the message
    connection->write(message.toString());
}

void CommunicationHandler::handleConnection() {
    connection = server.accept();
    if (!connection) {
        throw std::runtime_error("Failed to accept connection");
    }
    connectionCount++;

    while (isRunning) {
        read();
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

CommunicationHandler::~CommunicationHandler() {
    close();
}

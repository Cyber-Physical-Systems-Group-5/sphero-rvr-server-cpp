#include <string>
#include "../include/CommunicationHandler.hpp"
#include "../include/NetworkHelper.hpp"

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

std::string CommunicationHandler::read() {
    if (!connection) {
        return "";
    }

    std::vector<unsigned char> buffer(1024);
    std::string response;
    int bytesRead = 0;

    // Loop until no more data is available
    while ((bytesRead = connection->read(buffer)) > 0) {
        response.append(buffer.begin(), buffer.begin() + bytesRead);

        // Stop if the data received is less than the buffer size, meaning it's the last chunk
        if (bytesRead < buffer.size()) {
            break;
        }
    }

    return response;
}

void CommunicationHandler::write(const std::string &message) {
    // send the message
    connection->write(message);
}

void CommunicationHandler::handleConnection() {
    connection = server.accept();
    if (!connection) {
        throw std::runtime_error("Failed to accept connection");
    }
    connectionCount++;

    while (isRunning) {
        std::string message = read();
        if (!message.empty()) {
            messageQueue.push(message);
        }
    }
}

std::string CommunicationHandler::getLatestMessage() {
    if (messageQueue.empty()) {
        return "";
    }

    std::string message = messageQueue.front();
    messageQueue.pop();
    return message;
}

CommunicationHandler::~CommunicationHandler() {
    close();
}

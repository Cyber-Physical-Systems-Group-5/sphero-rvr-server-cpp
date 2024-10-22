#include <string>
#include "../include/CommunicationHandler.hpp"
#include "../include/NetworkHelper.hpp"

CommunicationHandler::CommunicationHandler(std::string &domain) : server(domain, 1) {
    connectionThread = std::jthread(&CommunicationHandler::acceptConnection, this);
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
    const auto bytesRead = connection->read(buffer);
    std::string response(buffer.begin(), buffer.begin() + bytesRead);
    return response;
}

void CommunicationHandler::write(const std::string &message) {
    // send the message
    connection->write(message);
}

void CommunicationHandler::acceptConnection() {
    connection = server.accept();
}


CommunicationHandler::~CommunicationHandler() {
    close();
}
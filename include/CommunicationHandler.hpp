//
// Created by kirk on 14/10/24.
//

#ifndef SPHERO_RVR_SERVER_CPP_COMMHANDLER_HPP
#define SPHERO_RVR_SERVER_CPP_COMMHANDLER_HPP

#include "simple_socket/TCPSocket.hpp"
#include "Message.hpp"
#include <vector>
#include <thread>
#include <queue>

using namespace simple_socket;

/**
 * @class CommunicationHandler
 * @brief Handles communication over Unix domain sockets, managing connections and providing
 *        read/write functionality. This class uses threading to handle incoming connections
 *        asynchronously.
 */
class CommunicationHandler {
private:
    TCPServer server;                               ///< The TCP server instance used for accepting connections.
    std::unique_ptr<SimpleConnection> connection;   ///< The active connection with the client.
    std::jthread connectionThread;                  ///< Thread for handling incoming connections.
    std::atomic<bool> isRunning{true};              ///< Flag to indicate whether the thread is active.
    std::queue<Message> messageQueue;           ///< Queue for storing received messages.

    void handleConnection();
    /**
     * @brief Reads data from the connected client and processes it.
     */
    void read();

    /**
     * @brief Closes the connection and stops the thread.
     */
    void close();



public:
    unsigned int connectionCount = 0;

    /**
     * @brief Constructs a CommunicationHandler and initializes a UnixDomainServer.
     *        Also spawns a thread to handle incoming connections.
     *
     * @param domain The domain path for the Unix domain socket.
     */
    explicit CommunicationHandler(uint16_t port);

    /**
     * @brief Reads the latest message from the message queue.
     *
     * @return The latest message from the queue, "" if empty.
     */
    Message getLatestMessage();

    // still need to implement this function
    void write(const Message& message);
    ~CommunicationHandler();
};


#endif //SPHERO_RVR_SERVER_CPP_COMMHANDLER_HPP

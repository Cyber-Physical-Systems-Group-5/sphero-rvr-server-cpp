//
// Created by kirk on 14/10/24.
//

#ifndef SPHERO_RVR_SERVER_CPP_COMMHANDLER_HPP
#define SPHERO_RVR_SERVER_CPP_COMMHANDLER_HPP

#include "simple_socket/UnixDomainSocket.hpp"
#include <vector>
#include <thread>

using namespace simple_socket;

/**
 * @class CommunicationHandler
 * @brief Handles communication over Unix domain sockets, managing connections and providing
 *        read/write functionality. This class uses threading to handle incoming connections
 *        asynchronously.
 */
class CommunicationHandler {
private:
    UnixDomainServer server;                        ///< The Unix domain server instance used for accepting connections.
    std::unique_ptr<SimpleConnection> connection;   ///< The active connection with the client.
    std::jthread connectionThread;                  ///< Thread for handling incoming connections.
    std::atomic<bool> isRunning{true};              ///< Flag to indicate whether the thread is active.

    void acceptConnection();

    void close();

public:
    unsigned int connectionCount = 0;

    /**
     * @brief Constructs a CommunicationHandler and initializes a UnixDomainServer.
     *        Also spawns a thread to handle incoming connections.
     *
     * @param domain The domain path for the Unix domain socket.
     */
    explicit CommunicationHandler(std::string &domain);

    // still need to implement this function
    void write(const std::string& message);

    /**
     * @brief Reads data from the connected client.
     *
     * @return A string containing the data read from the client. Returns an empty string if no connection exists.
     */
    std::string read();
    ~CommunicationHandler();
};


#endif //SPHERO_RVR_SERVER_CPP_COMMHANDLER_HPP

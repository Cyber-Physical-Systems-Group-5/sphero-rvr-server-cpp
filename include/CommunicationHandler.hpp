//
// Created by kirk on 14/10/24.
//

#ifndef SPHERO_RVR_SERVER_CPP_COMMHANDLER_HPP
#define SPHERO_RVR_SERVER_CPP_COMMHANDLER_HPP

#include "simple_socket/TCPSocket.hpp"
#include "simple_socket/UDPSocket.hpp"
#include "Message.hpp"
#include <vector>
#include <thread>
#include <queue>

using namespace simple_socket;

/**
 * @class CommunicationHandler
 * @brief Manages TCP communication with connected clients, supporting asynchronous read and write operations
 *        over TCP. This class is designed to handle incoming messages in a structured format
 *        and maintains an internal message queue for message retrieval.
 *
 * ## Message Format
 * Messages sent to this handler must be serialized using Protocol Buffers based on the defined `ProtoMessage` schema:
 * ```
 * syntax = "proto3";
 *
 * package proto;
 *
 * message ProtoMessage {
 *
 *   enum Direction {
 *     FORWARD = 0;
 *     BACKWARD = 1;
 *     RIGHT = 2;
 *     LEFT = 3;
 *   }
 *
 *   bytes image = 1;
 *   uint32 speed = 2;
 *   repeated Direction directions = 3;
 *
 * }
 * ```
 * Any message not conforming to this Protocol Buffers schema will be disregarded or may cause parsing errors.
 */
class CommunicationHandler {
private:
    std::string remoteAddress;
    uint16_t remotePort;
    TCPServer server;                                  ///< The TCP server instance used for accepting connections.
    std::unique_ptr<SimpleConnection> TCPconnection;   ///< The active TCP connection with the client.
    UDPSocket udpSocket;                               ///< The UDP socket instance used for accepting connections.

    std::jthread TCPconnectionThread;                  ///< Thread for handling incoming connections.
    std::jthread UDPconnectionThread;                  ///< Thread for handling incoming connections.
    std::atomic<bool> isRunning{true};                 ///< Flag to indicate whether the thread is active.

    std::queue<Message> commandQueue;                  ///< Queue for storing received command messages.
    std::queue<Message> imageQueue;                    ///< Queue for storing received image messages.

    /**
     * @brief Handles incoming TCP connections. This method is executed within the TCPconnection thread.
     */
    void handleTCPConnection();
    /**
     * @brief Handles incoming UDP connections. This method is executed within the UDPconnection thread.
     */
    void handleUDPConnection();
    /**
     * @brief Reads data from the connected client, processes it, and enqueues parsed messages.
     *        This method is executed within the TCPconnection thread.
     */
    void readTCP();

    /**
     * @brief Reads data from the connected client, processes it, and enqueues parsed messages.
     *        This method is executed within the TCPconnection thread.
     */
    void readUDP();

    /**
     * @brief Closes the TCPconnection and stops the thread.
     */
    void close();

    /**
     * @brief Converts a 32-bit integer from network byte order to host byte order.
     *
     * @param net The 32-bit integer in network byte order.
     * @return The 32-bit integer in host byte order.
     */
    uint32_t ntohl(uint32_t net);



public:
    unsigned int connectionCount = 0;

    /**
     * @brief Initializes a CommunicationHandler instance on a specified TCP port, setting up
     *        the server and spawning a thread to handle incoming connections asynchronously.
     *
     * @param port The TCP port to bind the server for incoming connections.
     */
    explicit CommunicationHandler(uint16_t TCPPort, uint16_t UDPPort, std::string remoteAddress, uint16_t remotePort);

    /**
     * @brief Retrieves the latest processed message from the internal message queue.
     *        If the queue is empty, returns an empty Message object.
     *
     * @return The latest available message in the queue.
     */
    Message getLatestCommandMessage();

    /**
     * @brief Retrieves the latest processed message from the internal message queue.
     *        If the queue is empty, returns an empty Message object.
     *
     * @return The latest available message in the queue.
     */
    Message getLatestImageMessage();

    /**
     * @brief Sends a message to the connected client. If no client is connected, the message is ignored.
     *
     * @param message The message to send over the active TCPconnection.
     */
    void write(const Message& message);

    /**
     * @brief Checks if there are any messages in the queue.
     *
     * @return True if there are messages in the queue, false otherwise.
     */
    bool hasMessages() const;

    ~CommunicationHandler();
};


#endif //SPHERO_RVR_SERVER_CPP_COMMHANDLER_HPP

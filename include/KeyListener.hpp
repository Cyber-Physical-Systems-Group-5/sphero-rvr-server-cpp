#ifndef RVR_SERVER_KEYHANDLER_CPP_H
#define RVR_SERVER_KEYHANDLER_CPP_H

#include <thread>
#include <queue>
#include <ncurses.h>
#include <condition_variable>
#include "../src/util/Message.hpp"

class KeyListener {
private:
    const int SPEED_INCREMENT = 5;
    const int MAX_SPEED = 255;
    int previousKey = ERR;
    uint8_t speed = 50;
    std::jthread keyDetectionThread;
    std::atomic<bool> isRunning{true};
    std::queue<Message> messageQueue;
    std::queue<char> keyQueue;
    std::mutex mtx;

    void detectKeys();

    void updateSpeed(int ch);

    void close();
public:
    std::condition_variable cv;

    std::mutex &getMtx();

    KeyListener();

    Message getMessage();

    char getKey();

    bool hasMessages() const;

    bool running() const;

    ~KeyListener();
};


#endif //RVR_SERVER_KEYHANDLER_CPP_H

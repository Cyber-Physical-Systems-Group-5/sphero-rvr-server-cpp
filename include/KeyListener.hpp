#ifndef RVR_SERVER_KEYHANDLER_CPP_H
#define RVR_SERVER_KEYHANDLER_CPP_H

#include <thread>
#include <queue>
#include <ncurses.h>
#include "../src/util/Message.hpp"

class KeyListener {
private:
    int previousKey = ERR;
    uint8_t speed = 100;
    std::jthread keyDetectionThread;
    std::atomic<bool> isRunning{true};
    std::unique_ptr<std::queue<Message>> messageQueue;

    void detectKeys();
public:
    KeyListener();

    Message getMessage();

    ~KeyListener();
};


#endif //RVR_SERVER_KEYHANDLER_CPP_H

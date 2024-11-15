#ifndef RVR_SERVER_KEYHANDLER_CPP_H
#define RVR_SERVER_KEYHANDLER_CPP_H

#include <thread>
#include <queue>
#include <ncurses.h>
#include "../src/util/Message.hpp"

class KeyHandler {
private:
    int previousKey = ERR;
    std::jthread keyDetectionThread;
    std::atomic<bool> isRunning{true};
    std::unique_ptr<std::queue<Message>> messageQueue;

    void detectKeys();
public:
    KeyHandler();

    Message getMessage();

    ~KeyHandler();
};


#endif //RVR_SERVER_KEYHANDLER_CPP_H

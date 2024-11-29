#include "../include/KeyListener.hpp"
#define ctrl(x)           ((x) & 0x1f)

KeyListener::KeyListener(): isRunning(true) {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    keyDetectionThread = std::jthread(&KeyListener::detectKeys, this);
}

KeyListener::~KeyListener() {
    close();
}

void KeyListener::close() {
    endwin();
}

void KeyListener::detectKeys() {
    int ch;
    while (isRunning) {
        Message message;
        if ((ch = getch()) != ERR) {
            keyQueue.push(ch);
            updateSpeed(ch);
            previousKey = ch;
            switch (ch) {
                case KEY_UP:
                    message.addDirection(Direction::FORWARD);
                    break;

                case KEY_DOWN:
                    message.addDirection(Direction::BACKWARD);
                    break;

                case KEY_LEFT:
                    message.addDirection(Direction::LEFT);
                    break;

                case KEY_RIGHT:
                    message.addDirection(Direction::RIGHT);
                    break;

                case 'w':
                    message.addCameraDirection(Direction::FORWARD);
                    break;

                case 'a':
                    message.addCameraDirection(Direction::LEFT);
                    break;

                case 's':
                    message.addCameraDirection(Direction::BACKWARD);
                    break;

                case 'd':
                    message.addCameraDirection(Direction::RIGHT);
                    break;

                case ctrl('c'):
                    isRunning = false;
                    close();
                    break;
            }
            message.setSpeed(speed);
            message.setType(Type::COMMAND);
            std::lock_guard<std::mutex> lock(mtx);
            messageQueue.push(message);
            cv.notify_one();
        }
    }
}

Message KeyListener::getMessage() {
    Message message;
    std::lock_guard<std::mutex> lock(mtx);
    if (!messageQueue.empty()) {
        message = messageQueue.front();
        messageQueue.pop();
    } else {
        throw std::runtime_error("No message available");
    }
    return message;
}

char KeyListener::getKey() {
    char key;
    std::lock_guard<std::mutex> lock(mtx);
    if (!keyQueue.empty()) {
        key = keyQueue.front();
        keyQueue.pop();
    } else {
        throw std::runtime_error("No key available");
    }
    return key;
}

void KeyListener::updateSpeed(int ch) {
    if (ch == previousKey && (ch == KEY_UP || ch == KEY_DOWN)) {
        speed = std::min(speed + SPEED_INCREMENT, MAX_SPEED);
    } else {
        speed = 0;
    }
}

std::mutex &KeyListener::getMtx() {
    return mtx;
}

bool KeyListener::hasMessages() const {
    return !messageQueue.empty() || !keyQueue.empty();
}

bool KeyListener::running() const {
    return isRunning;
}

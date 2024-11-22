#include "../include/KeyListener.hpp"


KeyListener::KeyListener(): messageQueue(std::make_unique<std::queue<Message>>()), isRunning(true) {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    keyDetectionThread = std::jthread(&KeyListener::detectKeys, this);
}

KeyListener::~KeyListener() {
    endwin();
}

void KeyListener::detectKeys() {
    int ch;
    while (isRunning) {
        Message message;
        if ((ch = getch()) != ERR) {
            switch (ch) {
                case KEY_UP:
                    previousKey = ch;
                    message.addDirection(Direction::FORWARD);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue->push(message);
                    break;

                case KEY_DOWN:
                    previousKey = ch;
                    message.addDirection(Direction::BACKWARD);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue->push(message);
                    break;

                case KEY_LEFT:
                    previousKey = ch;
                    message.addDirection(Direction::LEFT);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue->push(message);
                    break;

                case KEY_RIGHT:
                    previousKey = ch;
                    message.addDirection(Direction::RIGHT);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue->push(message);
                    break;

                case 'w':
                    previousKey = ch;
                    message.addCameraDirection(Direction::FORWARD);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue->push(message);
                    break;

                case 'a':
                    previousKey = ch;
                    message.addCameraDirection(Direction::LEFT);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue->push(message);
                    break;

                case 's':
                    previousKey = ch;
                    message.addCameraDirection(Direction::BACKWARD);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue->push(message);
                    break;

                case 'd':
                    previousKey = ch;
                    message.addCameraDirection(Direction::RIGHT);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue->push(message);
                    break;
            }
        }
    }
}

Message KeyListener::getMessage() {
    Message message;
    if (!messageQueue->empty()) {
        message = messageQueue->front();
        messageQueue->pop();
    } else {
        throw std::runtime_error("No message available");
    }
    return message;
}

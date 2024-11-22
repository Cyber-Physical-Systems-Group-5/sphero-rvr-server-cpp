#include "../include/KeyListener.hpp"


KeyListener::KeyListener(): isRunning(true) {
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
            updateSpeed(ch);
            previousKey = ch;
            switch (ch) {
                case KEY_UP:
                    message.addDirection(Direction::FORWARD);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue.push(message);
                    break;

                case KEY_DOWN:
                    message.addDirection(Direction::BACKWARD);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue.push(message);
                    break;

                case KEY_LEFT:
                    message.addDirection(Direction::LEFT);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue.push(message);
                    break;

                case KEY_RIGHT:
                    message.addDirection(Direction::RIGHT);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue.push(message);
                    break;

                case 'w':
                    message.addCameraDirection(Direction::FORWARD);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue.push(message);
                    break;

                case 'a':
                    message.addCameraDirection(Direction::LEFT);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue.push(message);
                    break;

                case 's':
                    message.addCameraDirection(Direction::BACKWARD);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue.push(message);
                    break;

                case 'd':
                    message.addCameraDirection(Direction::RIGHT);
                    message.setSpeed(speed);
                    message.setType(Type::COMMAND);
                    messageQueue.push(message);
                    break;
            }
        }
    }
}

Message KeyListener::getMessage() {
    Message message;
    if (!messageQueue.empty()) {
        message = messageQueue.front();
        messageQueue.pop();
    } else {
        throw std::runtime_error("No message available");
    }
    return message;
}

void KeyListener::updateSpeed(int ch) {
    if (ch == previousKey && (ch == KEY_UP || ch == KEY_DOWN)) {
        speed = std::min(speed + SPEED_INCREMENT, MAX_SPEED);
    } else {
        speed = 0;
    }
}

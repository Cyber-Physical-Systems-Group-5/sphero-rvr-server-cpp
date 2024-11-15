#include "../include/KeyHandler.hpp"


KeyHandler::KeyHandler(): messageQueue(std::make_unique<std::queue<Message>>()), isRunning(true) {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    keyDetectionThread = std::jthread(&KeyHandler::detectKeys, this);
}

KeyHandler::~KeyHandler() {
    endwin();
}

void KeyHandler::detectKeys() {
    int ch;
    while (isRunning) {
        Message message;
        if ((ch = getch()) != ERR) {
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
            }
            previousKey = ch;
        }
    }
}

Message KeyHandler::getMessage() {
    return getch();
}


    while ((ch = getch()) != '#') {
        switch(ch) {
            case KEY_UP: printw("\nUp");
                break;

            case KEY_DOWN: printw("\nDown");
                break;

            case KEY_LEFT: printw("\nLeft");
                break;

            case KEY_RIGHT: printw("\nRight");
                break;

            default: printw("%c", ch);
        }
    }
    refresh();
    getch();
    endwin();
}

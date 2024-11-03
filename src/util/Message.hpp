#ifndef RVR_SERVER_MESSAGE_HPP
#define RVR_SERVER_MESSAGE_HPP

#include "json.hpp"
#include "NetworkHelper.hpp"
#include "base64.hpp"

enum class Direction {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Message {
private:
    uint8_t speed;
    std::vector<Direction> directions;
    std::optional<std::string> image;
public:


    uint8_t getSpeed() const {
        return speed;
    }

    void setSpeed(uint8_t speed) {
        Message::speed = speed;
    }

    const std::vector<Direction> &getDirections() const {
        return directions;
    }

    void setDirections(const std::vector<Direction> &directions) {
        Message::directions = directions;
    }

    const std::optional<std::string> &getImage() const {
        return image;
    }

    void setImage(const std::optional<std::string> &image) {
        Message::image = image;
    }
    Message() = default;

    Message(uint8_t speed, std::vector<Direction> directions) : speed(speed), directions(std::move(directions)), image(std::nullopt) {}

    /**
     * @brief Construct a new Message object from a JSON string
     *
     * @param str JSON string
     * @return Message
     */
    static Message fromString(const std::string &str) {
        nlohmann::json json = nlohmann::json::parse(str);
        Message message;
        message.speed = json["speed"];
        for (const auto &direction : json["directions"]) {
            if (direction == "forward") {
                message.directions.push_back(Direction::FORWARD);
            } else if (direction == "backward") {
                message.directions.push_back(Direction::BACKWARD);
            } else if (direction == "left") {
                message.directions.push_back(Direction::LEFT);
            } else if (direction == "right") {
                message.directions.push_back(Direction::RIGHT);
            }
        }
        if (json.contains("image")) {
            message.image = base64::from_base64(json["image"].get<std::string>());
        } else {
            message.image = std::nullopt;
        }
        return message;
    }

    std::string toString() const {
        nlohmann::json json;
        json["speed"] = speed;
        for (const auto &direction : directions) {
            if (direction == Direction::FORWARD) {
                json["directions"].push_back("forward");
            } else if (direction == Direction::BACKWARD) {
                json["directions"].push_back("backward");
            } else if (direction == Direction::LEFT) {
                json["directions"].push_back("left");
            } else if (direction == Direction::RIGHT) {
                json["directions"].push_back("right");
            }
        }
        if (image.has_value()) {
            std::string base64Image = base64::to_base64(image.value());
            json["image"] = base64Image;
        }
        return json.dump();
    }



};

#endif //RVR_SERVER_MESSAGE_HPP

#ifndef RVR_SERVER_MESSAGE_HPP
#define RVR_SERVER_MESSAGE_HPP

#include <set>
#include <utility>
#include "json.hpp"
#include "base64.hpp"
#include "Image.pb.h"

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

    void setImageFromString(const std::string &image) {
        Message::image = image;
    }

    Message() = default;

    Message(uint8_t speed, std::vector<Direction> directions) : speed(speed), directions(std::move(directions)), image(std::nullopt) {}

    Message(uint8_t speed, std::vector<Direction> directions, std::optional<std::string> image) : speed(speed), directions(std::move(directions)), image(std::move(image)) {}

    /**
     * @brief Construct a new Message object from a JSON string
     *
     * @param str JSON string
     * @return Message
     */
    static Message fromJSONString(const std::string &str) {
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

    std::string toJSONString() const {
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

    bool operator==(const Message &rhs) const {
        // compare directions as a set
        std::set<Direction> lhsSet(directions.begin(), directions.end());
        std::set<Direction> rhsSet(rhs.directions.begin(), rhs.directions.end());
        return speed == rhs.speed &&
               lhsSet == rhsSet &&
               image == rhs.image;
    }

    bool operator!=(const Message &rhs) const {
        return !(rhs == *this);
    }

    static Message fromProto(const std::string& protoMsg) {
        proto::ProtoMessage message;
        bool success = message.ParseFromString(protoMsg);
        if (!success) {
            throw std::runtime_error("Failed to parse ProtoMessage");
        }

        auto& directions = message.directions();
        std::vector<Direction> directionsVector;
        for (const auto & direction : directions) {
            if (direction == "forward") {
                directionsVector.push_back(Direction::FORWARD);
            } else if (direction == "backward") {
                directionsVector.push_back(Direction::BACKWARD);
            } else if (direction == "left") {
                directionsVector.push_back(Direction::LEFT);
            } else if (direction == "right") {
                directionsVector.push_back(Direction::RIGHT);
            }
        }
        uint8_t speed = message.speed();
        std::optional<std::string> image;
        if (!message.image().empty()) {
            image = message.image();
        }

        return {speed, directionsVector, image};
    }

    std::string toProto() const {
        proto::ProtoMessage message;
        message.set_speed(speed);
        for (const auto & direction : directions) {
            if (direction == Direction::FORWARD) {
                message.add_directions("forward");
            } else if (direction == Direction::BACKWARD) {
                message.add_directions("backward");
            } else if (direction == Direction::LEFT) {
                message.add_directions("left");
            } else if (direction == Direction::RIGHT) {
                message.add_directions("right");
            }
        }
        if (image.has_value()) {
            message.set_image(image.value());
        }
        return message.SerializeAsString();
    }



};

#endif //RVR_SERVER_MESSAGE_HPP

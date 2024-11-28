#ifndef RVR_SERVER_MESSAGE_HPP
#define RVR_SERVER_MESSAGE_HPP

#include <set>
#include <utility>
#include "json.hpp"
#include "base64.hpp"
#include "Image.pb.h"

enum class Type {
    COMMAND,
    IMAGE,
    EMPTY
};
enum class Direction {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Message {
private:
    Type type;
    uint16_t distance;
    uint8_t speed;
    uint8_t battery_percentage;
    std::vector<Direction> directions;
    std::vector<Direction> cameraDirections;
    std::optional<std::string> image;
public:

    uint16_t getDistance() const {
        return distance;
    }

    void setDistance(uint16_t distance) {
        Message::distance = distance;
    }

    Type getType() const {
        return type;
    }

    void setType(Type type) {
        Message::type = type;
    }

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

    const std::vector<Direction> &getCameraDirections() const {
        return cameraDirections;
    }

    void setCameraDirections(const std::vector<Direction> &directions) {
        Message::cameraDirections = directions;
    }

    void addDirection(Direction direction) {
        directions.push_back(direction);
    }

    void addCameraDirection(Direction direction) {
        cameraDirections.push_back(direction);
    }

    const std::optional<std::string> &getImage() const {
        return image;
    }

    void setImageFromString(const std::string &image) {
        Message::image = image;
    }

    uint8_t getBatteryPercentage() const {
        return battery_percentage;
    }

    void setBatteryPercentage(uint8_t batteryPercentage) {
        battery_percentage = batteryPercentage;
    }

    Message() = default;

    Message(uint8_t speed, std::vector<Direction> directions) : speed(speed), directions(std::move(directions)), image(std::nullopt) {}

    Message(uint8_t speed, std::vector<Direction> directions, std::optional<std::string> image) : speed(speed), directions(std::move(directions)), image(std::move(image)) {}

    Message(Type type, uint16_t distance, uint8_t speed, std::vector<Direction> directions, std::vector<Direction> cameraDirections, std::optional<std::string> image, uint8_t battery_percentage) :
            type(type),
            distance(distance),
            speed(speed),
            directions(std::move(directions)),
            cameraDirections(std::move(cameraDirections)),
            image(std::move(image)),
            battery_percentage(battery_percentage){}

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
        message.distance = json["distance"];
        switch (json["type"].get<int>()) {
            case 0:
                message.type = Type::IMAGE;
                break;
            case 1:
                message.type = Type::COMMAND;
                break;
        }
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
        json["distance"] = distance;
        switch(type) {
            case Type::COMMAND:
                json["type"] = 1;
                break;
            case Type::IMAGE:
                json["type"] = 0;
                break;
        }
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
        std::set<Direction> lhsCameraSet(cameraDirections.begin(), cameraDirections.end());
        std::set<Direction> rhsCameraSet(rhs.cameraDirections.begin(), rhs.cameraDirections.end());
        return speed == rhs.speed &&
               lhsSet == rhsSet &&
               image == rhs.image &&
               type == rhs.type &&
               distance == rhs.distance &&
               lhsCameraSet == rhsCameraSet;
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
        auto& cameraDirections = message.camera_directions();

        std::vector<Direction> directionsVector;
        for (const auto & direction : directions) {
            if (direction == proto::ProtoMessage_Direction_FORWARD) {
                directionsVector.push_back(Direction::FORWARD);
            } else if (direction == proto::ProtoMessage_Direction_BACKWARD) {
                directionsVector.push_back(Direction::BACKWARD);
            } else if (direction == proto::ProtoMessage_Direction_LEFT) {
                directionsVector.push_back(Direction::LEFT);
            } else if (direction == proto::ProtoMessage_Direction_RIGHT) {
                directionsVector.push_back(Direction::RIGHT);
            }
        }

        std::vector<Direction> cameraDirectionsVector;
        for (const auto & direction : cameraDirections) {
            if (direction == proto::ProtoMessage_Direction_FORWARD) {
                cameraDirectionsVector.push_back(Direction::FORWARD);
            } else if (direction == proto::ProtoMessage_Direction_BACKWARD) {
                cameraDirectionsVector.push_back(Direction::BACKWARD);
            } else if (direction == proto::ProtoMessage_Direction_LEFT) {
                cameraDirectionsVector.push_back(Direction::LEFT);
            } else if (direction == proto::ProtoMessage_Direction_RIGHT) {
                cameraDirectionsVector.push_back(Direction::RIGHT);
            }
        }

        int type = message.type();
        Type messageType;
        switch (type) {
            case 0:
                messageType = Type::IMAGE;
                break;
            case 1:
                messageType = Type::COMMAND;
                break;
        }

        uint8_t speed = message.speed();
        uint8_t batteryPercentage = message.battery_percentage();
        uint16_t distance = message.distance();
        std::optional<std::string> image;
        if (!message.image().empty()) {
            image = message.image();
        }

        return {messageType, distance, speed, directionsVector, cameraDirectionsVector, image, batteryPercentage};
    }

    std::string toProto() const {
        proto::ProtoMessage message;
        message.set_speed(speed);
        message.set_distance(distance);
        message.set_battery_percentage(battery_percentage);
        switch (type) {
            case Type::COMMAND:
                message.set_type(proto::ProtoMessage_MessageType_COMMAND);
                break;
            case Type::IMAGE:
                message.set_type(proto::ProtoMessage_MessageType_IMAGE);
                break;
        }

        for (const auto & direction : directions) {
            switch (direction) {
                case Direction::FORWARD:
                    message.add_directions(proto::ProtoMessage_Direction_FORWARD);
                    break;
                case Direction::BACKWARD:
                    message.add_directions(proto::ProtoMessage_Direction_BACKWARD);
                    break;
                case Direction::LEFT:
                    message.add_directions(proto::ProtoMessage_Direction_LEFT);
                    break;
                case Direction::RIGHT:
                    message.add_directions(proto::ProtoMessage_Direction_RIGHT);
                    break;
            }
        }

        for (const auto & direction : cameraDirections) {
            switch (direction) {
                case Direction::FORWARD:
                    message.add_camera_directions(proto::ProtoMessage_Direction_FORWARD);
                    break;
                case Direction::BACKWARD:
                    message.add_camera_directions(proto::ProtoMessage_Direction_BACKWARD);
                    break;
                case Direction::LEFT:
                    message.add_camera_directions(proto::ProtoMessage_Direction_LEFT);
                    break;
                case Direction::RIGHT:
                    message.add_camera_directions(proto::ProtoMessage_Direction_RIGHT);
                    break;
            }
        }
        if (image.has_value()) {
            message.set_image(image.value());
        }
        return message.SerializeAsString();
    }

    std::string toString() const {
        std::string str;
        str += "Type: ";
        switch (type) {
            case Type::COMMAND:
                str += "COMMAND\n";
                break;
            case Type::IMAGE:
                str += "IMAGE\n";
                break;
            case Type::EMPTY:
                str += "EMPTY\n";
                break;
        }
        str += "Speed: " + std::to_string(speed) + "\n";
        str += "Distance: " + std::to_string(distance) + "\n";
        str += "Battery Percentage: " + std::to_string(battery_percentage) + "\n";
        str += "Directions: ";
        for (const auto &direction : directions) {
            switch (direction) {
                case Direction::FORWARD:
                    str += "FORWARD ";
                    break;
                case Direction::BACKWARD:
                    str += "BACKWARD ";
                    break;
                case Direction::LEFT:
                    str += "LEFT ";
                    break;
                case Direction::RIGHT:
                    str += "RIGHT ";
                    break;
            }
        }
        str += "\n";
        str += "Camera Directions: ";
        for (const auto &direction : cameraDirections) {
            switch (direction) {
                case Direction::FORWARD:
                    str += "FORWARD ";
                    break;
                case Direction::BACKWARD:
                    str += "BACKWARD ";
                    break;
                case Direction::LEFT:
                    str += "LEFT ";
                    break;
                case Direction::RIGHT:
                    str += "RIGHT ";
                    break;
            }
        }
        str += "\n";
        return str;
    }
};

#endif //RVR_SERVER_MESSAGE_HPP

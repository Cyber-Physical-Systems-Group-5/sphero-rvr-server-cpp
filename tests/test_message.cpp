#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include "Message.hpp"

std::string loadImage(std::filesystem::path path) {

    std::ifstream fileStream(path);
    if (!fileStream) {
        throw std::runtime_error("Failed to open file at: " + path.string());
    }

    std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    return fileContent;
}

TEST_CASE("Message from string", "[message]") {
    // Load test image
    std::filesystem::path imagePath = IMAGE_PATH;
    std::string image = loadImage(imagePath);

    Message message = Message::fromJSONString("{\"speed\": 100, \"directions\": [\"forward\", \"left\"], \"MessageType\": 0}");
    message.setImageFromString(image);

    REQUIRE(message.getSpeed() == 100);
    auto directions = message.getDirections();
    REQUIRE(directions.size() == 2);
    REQUIRE(directions[0] == Direction::FORWARD);
    REQUIRE(directions[1] == Direction::LEFT);
    REQUIRE(message.getImage().has_value());
    REQUIRE(message.getImage().value() == image);
    REQUIRE(message.getType() == Type::IMAGE);
}

TEST_CASE("Message to string", "[message]") {
    // Load test image
    std::filesystem::path imagePath = IMAGE_PATH;
    std::string image = loadImage(imagePath);

    Message message = Message(100, {Direction::FORWARD, Direction::LEFT}, Type::IMAGE);
    message.setImageFromString(image);

    std::string str = message.toJSONString();
    Message parsedMessage = Message::fromJSONString(str);

    REQUIRE(parsedMessage.getSpeed() == 100);
    auto directions = parsedMessage.getDirections();
    REQUIRE(directions.size() == 2);
    REQUIRE(directions[0] == Direction::FORWARD);
    REQUIRE(directions[1] == Direction::LEFT);
    REQUIRE(parsedMessage.getImage().has_value());
    REQUIRE(parsedMessage.getImage().value() == image);
    REQUIRE(parsedMessage.getType() == Type::IMAGE);
}

TEST_CASE("Message to proto", "[message]") {
    // Load test image
    std::filesystem::path imagePath = IMAGE_PATH;
    std::string image = loadImage(imagePath);

    Message message = Message(100, {Direction::FORWARD, Direction::LEFT}, Type::IMAGE);
    message.setImageFromString(image);

    std::string str = message.toProto();
    Message newMessage = Message::fromProto(str);

    REQUIRE(newMessage.getSpeed() == 100);
    auto directions = newMessage.getDirections();
    REQUIRE(directions.size() == 2);
    REQUIRE(directions[0] == Direction::FORWARD);
    REQUIRE(directions[1] == Direction::LEFT);
    REQUIRE(newMessage.getImage().has_value());
    REQUIRE(newMessage.getImage().value() == image);
    REQUIRE(newMessage.getType() == Type::IMAGE);
}
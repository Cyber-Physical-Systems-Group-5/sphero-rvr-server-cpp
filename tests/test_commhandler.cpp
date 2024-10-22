#include "CommunicationHandler.hpp"
#include <catch2/catch_test_macros.hpp>
#include <thread>

TEST_CASE("CommunicationHandler read/write") {
    std::string domain = "test";


    std::thread serverThread([&] {
        CommunicationHandler server(domain);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto response = server.read();
        REQUIRE(response == "test_message");

    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::thread clientThread([&] {

        UnixDomainClientContext client;
        const auto conn = client.connect(domain);
        REQUIRE(conn);

        std::string message = "test_message";
        conn->write(message);
    });

    clientThread.join();
    serverThread.join();
}


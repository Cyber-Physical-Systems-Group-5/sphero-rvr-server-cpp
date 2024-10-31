#include "CommunicationHandler.hpp"
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <condition_variable>

TEST_CASE("CommunicationHandler read/write") {
    uint16_t port = 8000;
    std::condition_variable cv;
    std::mutex cvMutex;
    bool serverReady = false;

    std::string message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec viverra rutrum lectus ac luctus. Quisque cursus erat et dui tincidunt, porta cursus leo vehicula. Sed eros odio, dignissim non erat tristique, eleifend suscipit nulla. Sed congue magna elit, sit amet suscipit sapien aliquet ac. Maecenas id magna mollis, elementum tortor non, congue nulla. Sed sed odio ante. Vivamus fringilla enim felis, eu lacinia ante cursus quis. Etiam convallis ultrices pharetra. Aliquam lectus lorem, mollis a eleifend sed, volutpat ut turpis. Vivamus suscipit libero vehicula velit varius, eget condimentum arcu pharetra. Nullam elementum risus quis arcu vehicula, vel aliquet enim efficitur. Vestibulum consectetur sapien lacus, nec aliquam sapien aliquet luctus. Cras at nulla urna.\n"
                          "Donec auctor magna ut cursus ultrices. Phasellus egestas convallis ex a sollicitudin. Vestibulum eget dui mi. Curabitur et velit erat. Proin gravida neque eget libero pharetra tempor. Donec vitae ultricies neque, sit amet rhoncus enim. Quisque at mi pretium, molestie ipsum eu, tempor mauris. Curabitur pulvinar ornare nibh, sit amet mattis arcu auctor at. Suspendisse luctus blandit nisi sed eleifend. Vivamus lacinia lacus ultrices tempor consequat.\n"
                          "Vivamus egestas volutpat massa quis imperdiet. Vestibulum euismod tortor eget tincidunt viverra. In lacus eros, euismod consectetur augue vel, lobortis ultrices orci. Quisque ut tortor vel massa condimentum cursus. Ut accumsan quis ex sit amet dignissim. Sed sed nibh eget lorem fermentum ultricies rutrum vel nunc. Sed ut lectus mauris. Curabitur quis egestas lacus. Pellentesque eu sem at nunc porta facilisis eu quis enim.\n"
                          "Nullam vitae felis porttitor, convallis felis vitae, tempus est. In auctor purus vestibulum erat ornare, id eleifend turpis condimentum. Sed id scelerisque sapien. Suspendisse hendrerit nulla mauris, vestibulum porta nisl viverra eu. Integer finibus sit amet ante sed placerat. Nunc ultrices mauris a mi vehicula, ac scelerisque mi venenatis. Maecenas ac rhoncus eros. Nam sed pharetra. ";

    std::thread serverThread([&] {
        CommunicationHandler server(port);

        // Signal that the server is ready
        {
            std::lock_guard<std::mutex> lock(cvMutex);
            serverReady = true;
        }
        cv.notify_one();

        // Wait 5 ms for client to connect
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Wait for and read client message
        auto response = server.getLatestMessage();
        CHECK(response == message);
    });

    // Wait for server to be ready
    {
        std::unique_lock<std::mutex> lock(cvMutex);
        cv.wait(lock, [&serverReady] { return serverReady; });
    }

    // Start client thread only after server is ready
    std::thread clientThread([&] {
        TCPClientContext client;
        const auto conn = client.connect("127.0.0.1", port);
        REQUIRE(conn);
        conn->write(message);
    });

    clientThread.join();
    serverThread.join();
}
# Define the first test executable for commhandler
add_executable(commhandler_test test_commhandler.cpp)
add_test(NAME commhandler_test COMMAND commhandler_test)
target_include_directories(commhandler_test
        PRIVATE ${simple_socket_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(commhandler_test PRIVATE
        comm_handler
        Catch2::Catch2WithMain
        simple_socket
)

# Define the second test executable for message
add_executable(message_test test_message.cpp)
add_test(NAME message_test COMMAND message_test)
target_include_directories(message_test
        PRIVATE ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/src/util
)
target_link_libraries(message_test PRIVATE
        Catch2::Catch2WithMain
)

# Set environment variable for testing
target_compile_definitions(commhandler_test PRIVATE IMAGE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/Lenna.png")
target_compile_definitions(message_test PRIVATE IMAGE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/Lenna.png")
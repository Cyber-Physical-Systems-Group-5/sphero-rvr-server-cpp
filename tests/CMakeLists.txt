add_executable(comm_handler_test test_commhandler.cpp)
add_test(NAME comm_handler_test COMMAND comm_handler_test)

target_include_directories(comm_handler_test
        PRIVATE ${simple_socket_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(comm_handler_test PRIVATE
        comm_handler
        Catch2::Catch2WithMain
        simple_socket
)
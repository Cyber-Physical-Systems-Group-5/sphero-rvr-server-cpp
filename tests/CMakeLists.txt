# Check if PROTOBUF_INCLUDE_DIR is set
if(NOT DEFINED PROTOBUF_INCLUDE_DIR)
    message(FATAL_ERROR "PROTOBUF_INCLUDE_DIR is not set. Please define the path to the Protobuf include directory.")
else()
    include_directories(${PROTOBUF_INCLUDE_DIR})
endif()

# Check if PROTOBUF_LIB_DIR is set
if(NOT DEFINED PROTOBUF_LIB_DIR)
    message(FATAL_ERROR "PROTOBUF_LIB_DIR is not set. Please define the path to the Protobuf library directory.")
else()
    link_directories(${PROTOBUF_LIB_DIR})
endif()


# Add protobuf library to the executable target
find_package(protobuf REQUIRED CONFIG)
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${CMAKE_SOURCE_DIR}/tests/image.proto") # Update proto file path


add_executable(comm_handler_test
        test_commhandler.cpp
        image.pb.cc
        image.pb.h
)
target_compile_definitions(comm_handler_test PRIVATE PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

add_test(NAME comm_handler_test COMMAND comm_handler_test)


target_include_directories(comm_handler_test
        PRIVATE ${simple_socket_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/tests
)

target_link_libraries(comm_handler_test PRIVATE
        comm_handler
        Catch2::Catch2WithMain
        simple_socket
        protobuf::libprotobuf
)
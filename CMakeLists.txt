cmake_minimum_required(VERSION 3.15)
include(FetchContent)
project(rvr_server)

option(BUILD_TESTS "Build tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(OpenCV_DIR "$ENV{OpenCV_DIR}")


find_package(protobuf CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(Curses REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )

add_subdirectory(protobuf)

add_subdirectory(src)

if (BUILD_TESTS)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)

    enable_testing()
    add_subdirectory(tests)
endif ()

set(SIMPLE_SOCKET_BUILD_TESTS OFF)
FetchContent_Declare(
        simple_socket
        GIT_REPOSITORY https://github.com/markaren/SimpleSocket.git
        GIT_TAG e0e989bdd52137d743a4599e25167d5681a0de00
)
FetchContent_MakeAvailable(simple_socket)


add_executable(rvr_server main.cpp)
target_include_directories(rvr_server PRIVATE
        PRIVATE ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/src/util
)
target_link_libraries(rvr_server PRIVATE comm_handler)
target_link_libraries(rvr_server PRIVATE simple_socket)
target_link_libraries(rvr_server PRIVATE proto_msg)
target_link_libraries(rvr_server PRIVATE ${OpenCV_LIBRARIES})
target_link_libraries(rvr_server PRIVATE ${CURSES_LIBRARIES})

file(COPY "data/" DESTINATION "${CMAKE_BINARY_DIR}/data")

target_compile_definitions(rvr_server PRIVATE YOLO_CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/yolov7-tiny.cfg")
target_compile_definitions(rvr_server PRIVATE YOLO_WEIGHTS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/yolov7-tiny.weights")
target_compile_definitions(rvr_server PRIVATE YOLO_CLASSES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/coco.names")
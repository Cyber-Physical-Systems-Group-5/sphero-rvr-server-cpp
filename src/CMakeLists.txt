set(includeDir "${PROJECT_SOURCE_DIR}/include")
set(srcDir "${PROJECT_SOURCE_DIR}/src")
set(utilDir "${PROJECT_SOURCE_DIR}/src/util")

set(headers
        "${includeDir}/CommunicationHandler.hpp"
        "${includeDir}/json.hpp"
        "${includeDir}/KeyListener.hpp"
        "${srcDir}/util/Message.hpp"
)

set(sources
        "${srcDir}/CommunicationHandler.cpp"
        "${srcDir}/KeyListener.cpp"
)

add_library(comm_handler "${headers}" "${sources}")

target_include_directories(comm_handler
        PRIVATE "${includeDir}"
        PRIVATE "${srcDir}"
        PRIVATE ${simple_socket_SOURCE_DIR}/include
        PRIVATE ${utilDir}
)

target_link_libraries(comm_handler PRIVATE simple_socket proto_msg)


add_executable(key_handler KeyListener.cpp)
target_link_libraries(key_handler PRIVATE curses)
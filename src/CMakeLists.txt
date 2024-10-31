set(includeDir "${PROJECT_SOURCE_DIR}/include")

set(headers
        "${includeDir}/CommunicationHandler.hpp"
        "${includeDir}/NetworkHelper.hpp"
)

set(sources
        "CommunicationHandler.cpp"
)

add_library(comm_handler "${headers}" "${sources}")
target_include_directories(comm_handler
        PRIVATE "${includeDir}"
        PRIVATE ${simple_socket_SOURCE_DIR}/include
)
target_link_libraries(comm_handler PRIVATE simple_socket)
set(includeDir "${PROJECT_SOURCE_DIR}/include")
set(srcDir "${PROJECT_SOURCE_DIR}/src")
set(utilDir "${PROJECT_SOURCE_DIR}/src/util")

set(headers
        "${includeDir}/CommunicationHandler.hpp"
        "${includeDir}/NetworkHelper.hpp"
        "${includeDir}/json.hpp"
        "${srcDir}/util/Message.hpp"
)

set(sources
        "${srcDir}/CommunicationHandler.cpp"
)

add_library(comm_handler "${headers}" "${sources}")

target_include_directories(comm_handler
        PRIVATE "${includeDir}"
        PRIVATE "${srcDir}"
        PRIVATE ${simple_socket_SOURCE_DIR}/include
        PRIVATE ${utilDir}
)

target_link_libraries(comm_handler PRIVATE simple_socket)
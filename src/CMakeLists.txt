set(includeDir "${PROJECT_SOURCE_DIR}/include")

set(headers
        "${includeDir}/CommHandler.hpp"
        "${includeDir}/NetworkHelper.hpp"
)

set(sources
        "CommHandler.cpp"
)

add_library(comm_handler "${headers}" "${sources}")
target_include_directories(comm_handler
        PRIVATE "${includeDir}"
        PRIVATE ${simple_socket_SOURCE_DIR}/include
)
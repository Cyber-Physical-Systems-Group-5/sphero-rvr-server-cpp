# C++ Project with Protobuf and OpenCV

This project involves a C++ application that communicates with a Raspberry Pi-based robot (Sphero RVR) and processes images using OpenCV. The project is built using CMake and relies on Protobuf for message serialization.

## Requirements

- **Protobuf runtime** and **protoc**: Protobuf is used for serializing data in this project. You need to install both the runtime and the compiler (`protoc`).
    - Follow the official guide to install Protobuf:
        - [Protobuf Installation Guide](https://github.com/protocolbuffers/protobuf/tree/main/src)

- **OpenCV**: This project uses OpenCV for image processing.

- **vcpkg**: This project uses vcpkg for managing dependencies. You need to set up the toolchain for CMake using vcpkg.

## Dependencies

- **CMake**
- **Protobuf**
- **OpenCV**

## Setup Instructions

1. **Install Dependencies**:
    - Install **Protobuf** and **OpenCV** following the respective installation guides.
    - Set up **vcpkg** by following the official instructions: https://github.com/microsoft/vcpkg

2. **Clone the Repository**
3. **Configure the Project with CMake**: Set the following CMake flags during the configuration process:
    ```
    cmake -DCMAKE_TOOLCHAIN_FILE= \
    -DPROTOBUF_INCLUDE_DIR= \
    -DPROTOBUF_LIB_DIR= \
    -DCMAKE_PREFIX_PATH= \
    .
    ```
4. **Build the Project**: Run `make` to build the project.

## Additional Notes
- Make sure that the Protobuf compiler (`protoc`) is in your PATH.
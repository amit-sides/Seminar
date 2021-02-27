
cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER gcc-10)
set(CMAKE_CXX_COMPILER g++-10)
set(CMAKE_CXX_STANDARD 20)

set(PROJECT_NAME Client)
set(DEPENDENCIES wolfssl)

project(${PROJECT_NAME})
set(SOURCE_FILES    main.cpp
                    TLSSocket.cpp   TLSSocket.h
                    messages.h
                    client.cpp client.h
                    example.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${DEPENDENCIES})
add_subdirectory(dependencies)
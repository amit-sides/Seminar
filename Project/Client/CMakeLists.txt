
cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER gcc-10)
set(CMAKE_CXX_COMPILER g++-10)
set(CMAKE_CXX_STANDARD 20)
set(PROJECT_FLAGS "                         \
                   -D_FORTIFY_SOURCE=2      \
                   -D_GLIBCXX_ASSERTIONS    \
                   -fpie                    \
                   -Wl,-pie                 \
                   -fpic                    \
                   -fstack-clash-protection \
                   -fstack-protector-strong \
                   -O2                      \
                   -Wall                    \
                   -Wpedantic               \
                  ")

set(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS} ${PROJECT_FLAGS}")
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} ${PROJECT_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${PROJECT_FLAGS}")

set(PROJECT_NAME Client)
set(DEPENDENCIES wolfssl)

project(${PROJECT_NAME})
set(SOURCE_FILES    main.cpp
                    tlssocket.cpp tlssocket.h
                    messages.h
                    client.cpp client.h)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${DEPENDENCIES})
add_subdirectory(dependencies)
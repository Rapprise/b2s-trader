cmake_minimum_required(VERSION 3.5.1)
project(signature_encryptor)

option(ENABLE_TESTS "Build all tests." OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-std=gnu++17)
add_definitions(-fPIC)

file(GLOB ENCRYPTOR_SRC
        "include/*.h"
        "src/*.cpp"
        "resources/*.h")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(signature_encryptor
        src/main.cpp)

target_link_libraries(signature_encryptor ${OPENSSL_LIBRARIES})

if(ENABLE_TESTS)
    add_subdirectory(unit_tests)
	set(UNIT_TESTS ${UNIT_TESTS} PARENT_SCOPE)
endif()

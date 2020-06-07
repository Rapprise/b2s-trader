cmake_minimum_required(VERSION 3.5.1)
project(stock_exchange)

option(ENABLE_TESTS "Build all tests." OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-std=gnu++17)
add_definitions(-fPIC)

find_path(Poco_INCLUDE_DIRS Poco/Poco.h)

include_directories(${Poco_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIR})


file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB INCLUDE_FILES include/*.h)

set(RESOURCES_FILES
		resources/resources.h include/huobi_api_signature.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} STATIC ${INCLUDE_FILES} ${RESOURCES_FILES} ${SOURCE_FILES})

if(ENABLE_TESTS)
    add_subdirectory(unit-test)
	set(UNIT_TESTS ${UNIT_TESTS} PARENT_SCOPE)
endif()

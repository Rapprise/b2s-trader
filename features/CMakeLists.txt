cmake_minimum_required(VERSION 3.5.1)
project(features)

option(ENABLE_TESTS "Build all tests." OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-std=gnu++17)
add_definitions(-fPIC)

find_path(Poco_INCLUDE_DIRS Poco/Poco.h)

include_directories(${Poco_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})

set(INCLUDE_FILES
        include/stop_loss_announcer.h
        include/telegram_announcer.h
        include/license.h
    )

set(RESOURCES_FILES
        resources/commands.h
        resources/resources.h
        resources/feature_utils.h
    )

set(SOURCE_FILES
        src/telegram_announcer.cpp
        src/stop_loss_announcer.cpp
        src/license.cpp
        )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

if(ENABLE_TESTS)
    add_subdirectory(unit-tests)
	set(UNIT_TESTS ${UNIT_TESTS} PARENT_SCOPE)
endif()

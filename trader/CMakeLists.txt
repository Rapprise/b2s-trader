cmake_minimum_required(VERSION 3.5.1)
project(trader)

set(CMAKE_AUTOMOC ON)

option(ENABLE_TESTS "Build all tests." OFF)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Poco_INCLUDE_DIRS})
include_directories(${SQLITE3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/strategies)
include_directories(${CMAKE_SOURCE_DIR}/stocks_exchange)
include_directories(${CMAKE_SOURCE_DIR}/model)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
    add_definitions(-fPIC)
endif()


set(INCLUDE_FILES
    include/app_controller.h
    include/app_stats_updater.h
    include/app_chart_updater.h
    include/trading_manager.h
    include/trading_buying_strategy_processor.h
    include/trading_selling_strategy_processor.h
    include/trading_message_sender.h
    include/trader.h)

set(SOURCE_FILES
    src/trading_manager.cpp
    src/trading_buying_strategy_processor.cpp
    src/trading_selling_strategy_processor.cpp
    src/app_controller.cpp
    src/app_stats_updater.cpp
    src/app_chart_updater.cpp
    src/trader.cpp
    src/trading_message_sender.cpp)

qt5_wrap_cpp(INCLUDE_GENERATED_FILES ${INCLUDE_FILES})

add_library(${PROJECT_NAME} STATIC ${INCLUDE_FILES} ${SOURCE_FILES} ${INCLUDE_GENERATED_FILES})

if(ENABLE_TESTS)
    add_subdirectory(unit-test)
    set(UNIT_TESTS ${UNIT_TESTS} PARENT_SCOPE)
endif()
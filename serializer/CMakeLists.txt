cmake_minimum_required (VERSION 2.8.12.2)
project (serializer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Poco_INCLUDE_DIRS})

set(INCLUDE_FILES
	include/app_settings_json_serializer.h
	include/feature_json_serializer.h
	include/strategy_json_serializer.h
	include/trade_config_json_serializer.h)

set(SOURCE_FILES
        src/strategy_json_serializer.cpp
        src/trade_config_json_serializer.cpp
        src/feature_json_serializer.cpp
		src/app_settings_json_serializer.cpp)

add_library(${PROJECT_NAME} STATIC ${INCLUDE_FILES} ${SOURCE_FILES})

if(ENABLE_TESTS)
    add_subdirectory(unit-tests)
	set(UNIT_TESTS ${UNIT_TESTS} PARENT_SCOPE)
endif()
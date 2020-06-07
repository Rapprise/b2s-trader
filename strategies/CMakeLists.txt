cmake_minimum_required(VERSION 3.0)

project(strategies)

option(ENABLE_TESTS "Build all tests." OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(${Qt5Widgets_INCLUDE_DIRS})

file(GLOB STRATEGIES_LIB_SOURCES
		"include/*.h"
		"include/bollinger_bands/*.h"
		"include/bollinger_bands_advance/*.h"
		"include/rsi/*.h"
		"include/simple_moving_average/*.h"
		"include/exponential_moving_average/*.h"
		"include/moving_average/*.h"
		"include/moving_averages_crossing/*.h"
		"include/stochastic_oscillator/*.h"
		"include/macd/*.h"
		"include/oscillator_base/*.h"
		"src/*.cpp"
		"src/bollinger_bands/*.cpp"
		"src/bollinger_bands_advance/*.cpp"
		"src/rsi/*.cpp"
		"src/simple_moving_average/*.cpp"
		"src/exponential_moving_average/*.cpp"
		"src/moving_average/*.cpp"
		"src/moving_averages_crossing/*.cpp"
		"src/stochastic_oscillator/*.cpp"
		"src/macd/*.cpp"
		"src/oscillator_base/*.cpp"
		"resources/*.h"
)

add_library(strategies STATIC ${STRATEGIES_LIB_SOURCES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

if(ENABLE_TESTS)
	add_subdirectory(unit_test)
	set(UNIT_TESTS ${UNIT_TESTS} PARENT_SCOPE)
endif()


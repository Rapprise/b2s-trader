cmake_minimum_required(VERSION 3.0)

project(strategies_unit_tests)

file(GLOB STRATEGIES_TESTS_SOURCES
		"*.h"
		"*.cpp"
)

find_package (OpenSSL REQUIRED)

add_executable(strategies_unit_tests ${STRATEGIES_TESTS_SOURCES})

if(WIN32)
	set_property(TARGET strategies_unit_tests PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${BINARY_DEPEND_PATH}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(strategies_unit_tests gtest gtest_main gmock ${PTHREAD} strategies ${OPENSSL_LIBRARIES} Qt5::Core)

if(WIN32)
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/strategies_unit_tests PARENT_SCOPE)
else()
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/strategies_unit_tests PARENT_SCOPE)
endif()


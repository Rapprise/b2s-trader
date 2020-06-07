cmake_minimum_required(VERSION 3.0)

project(database_unit_tests)

file(GLOB DATABASE_TESTS_SOURCES
        "*.h"
        "*.cpp"
        )

add_executable(database_unit_tests ${DATABASE_TESTS_SOURCES})



if(WIN32)	
	set_property(TARGET database_unit_tests PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${BINARY_DEPEND_PATH}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(database_unit_tests gtest gtest_main gmock ${PTHREAD} database model ${SQLITE_LIB} Qt5::Widgets)

if(WIN32)
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/database_unit_tests PARENT_SCOPE)
else()
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/database_unit_tests PARENT_SCOPE)
endif()





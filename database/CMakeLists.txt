cmake_minimum_required (VERSION 2.8.12.2)
project (database)

include_directories(${SQLITE3_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES
        src/database.cpp)

set(INCLUDE_FILES
		include/database.h)

add_library(${PROJECT_NAME} STATIC ${INCLUDE_FILES} ${SOURCE_FILES})

if(ENABLE_TESTS)
    add_subdirectory(unit-test)
    set(UNIT_TESTS ${UNIT_TESTS} PARENT_SCOPE)
endif()

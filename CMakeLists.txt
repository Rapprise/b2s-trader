cmake_minimum_required(VERSION 3.5.1)
project(b2s_trader)

enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS_RELEASE "/MD")
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
endif()

if(UNIX)
    add_definitions(-fPIC)
    add_definitions("-DQT_STATICPLUGIN")
    set(PTHREAD pthread)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(UNIX AND NOT APPLE)
    set(LIB_ENDING "so")
    set(DEBUG_LIB_ENDING "")
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(DEBUG_LIB_ENDING "d")
    endif(CMAKE_BUILD_TYPE MATCHES Debug)
endif()

if(APPLE)
    set(LIB_ENDING "dylib")
    set(DEBUG_LIB_ENDING "")
endif()

if(WIN32)
    set(LIB_ENDING "lib")
    set(DEBUG_LIB_ENDING "")
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set(DEBUG_LIB_ENDING "d")
    endif (CMAKE_BUILD_TYPE MATCHES Debug)
endif()

if(UNIX)
    set(POCO_LIB_DIR "/usr/local/lib")
    set(SQLITE_LIB sqlite3)
    set(POCO_LIBS        "${POCO_LIB_DIR}/libPocoNet${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/libPocoUtil${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/libPocoXML${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/libPocoJSON${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/libPocoFoundation${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/libPocoNetSSL${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/libPocoCrypto${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "pthread")
endif()

if(WIN32)
    set(POCO_LIBS        "${POCO_LIB_DIR}/PocoNet${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/PocoUtil${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/PocoXML${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/PocoFoundation${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/PocoJSON${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/PocoNetSSL${DEBUG_LIB_ENDING}.${LIB_ENDING}"
        "${POCO_LIB_DIR}/PocoCrypto${DEBUG_LIB_ENDING}.${LIB_ENDING}")

endif()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${SQLITE3_INCLUDE_DIRS})
include_directories (${OPENSSL_INCLUDE_DIR})
include_directories (${CURL_INCLUDE_DIR})

message("OPENSSL_INCLUDE_DIR = ${OPENSSL_INCLUDE_DIR}")
message("OPENSSL_LIB_DIR = ${OPENSSL_LIB_DIR}")
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message("Poco_INCLUDE_DIRS = ${Poco_INCLUDE_DIRS}")
message("POCO_LIB_DIR = ${POCO_LIB_DIR}")
message("CURL_INCLUDE_DIR = ${CURL_INCLUDE_DIR}")
message("CURL_LIBRARIES = ${CURL_LIBRARIES}")
message("SQLITE3_INCLUDE_DIRS = ${SQLITE3_INCLUDE_DIRS}")
message("SQLITE_LIB = ${SQLITE_LIB}")
message("GTEST_DIR = ${GTEST_DIR}")
message("BINARY_DEPEND_PATH = ${BINARY_DEPEND_PATH}")

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Charts REQUIRED)
find_package(Qt5Network REQUIRED)
find_package (OpenSSL REQUIRED)
find_package (CURL REQUIRED)

add_subdirectory(stocks_exchange)
add_subdirectory(strategies)
add_subdirectory(model)
add_subdirectory(view)
add_subdirectory(trader)
add_subdirectory(database)
add_subdirectory(serializer)
add_subdirectory(features)
add_subdirectory(signature_encryptor)

if(ENABLE_TESTS)
    if(WIN32)
        set(gtest_force_shared_crt ON)
    endif()
    add_subdirectory(${GTEST_DIR})
endif()

set(CMAKE_AUTORCC ON)

set(SOURCE_FILES trader/main.cpp theme/qdarkstyle/style.qrc resources/images_resources.qrc )

if(WIN32)
    add_executable(b2s_trader WIN32 ${SOURCE_FILES} resources/application_icons/windows_icon_resources.rc)
    set_property(TARGET b2s_trader PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${BINARY_DEPEND_PATH}")
elseif(APPLE)
    set(ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources/application_icons/macos_app_icon.icns common/exceptions/stock_exchange_exception/incorrect_json_exception.h)
    set(MACOSX_BUNDLE_ICON_FILE macos_app_icon.icns)
    set(MACOSX_BUNDLE_BUNDLE_NAME b2s_trader)
    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    add_executable(b2s_trader MACOSX_BUNDLE ${ICON_FILE} ${SOURCE_FILES})
else()
    add_executable(b2s_trader ${SOURCE_FILES})
endif()

target_link_libraries(
    b2s_trader
    trader
    strategies
    stock_exchange
    view
    model
    serializer
    database
    features
    ${POCO_LIBS}
    Qt5::Widgets
    Qt5::Core
    Qt5::Charts
    Qt5::Network
    ${SQLITE_LIB}
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES})

if(WIN32)
    add_custom_target(all_tests DEPENDS ALL_BUILD)
else()
    add_custom_target(all_tests DEPENDS ${UNIT_TESTS} signature_encryptor)
endif()

foreach(UNIT_TEST ${UNIT_TESTS})
    message("UT = ${UNIT_TEST}")
    LIST( APPEND unit_test_command COMMAND ${UNIT_TEST})
endforeach()

add_custom_command(
    TARGET all_tests
    COMMENT "Run all unit tests"
    POST_BUILD
    COMMAND ${unit_test_command}
)



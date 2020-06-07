cmake_minimum_required(VERSION 3.0)

project(trader_unit_tests)

file(GLOB TRADER_TEST_SOURCES
    "*.h"
    "*.cpp"
    "fake/*.h"
    "fake/*.cpp")

find_package (OpenSSL REQUIRED)

if(WIN32)
	set(POCO_LIBS        "${POCO_LIB_DIR}/PocoNet${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoUtil${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoXML${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoFoundation${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoJSON${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoNetSSL${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoCrypto${DEBUG_LIB_ENDING}.${LIB_ENDING}")
endif()

if(UNIX)
	set(POCO_LIB_DIR     "/usr/local/lib")
	set(POCO_LIBS        "${POCO_LIB_DIR}/libPocoNet${DEBUG_LIB_ENDING}.${LIB_ENDING}"
			"${POCO_LIB_DIR}/libPocoUtil${DEBUG_LIB_ENDING}.${LIB_ENDING}"
			"${POCO_LIB_DIR}/libPocoXML${DEBUG_LIB_ENDING}.${LIB_ENDING}"
			"${POCO_LIB_DIR}/libPocoJSON${DEBUG_LIB_ENDING}.${LIB_ENDING}"
			"${POCO_LIB_DIR}/libPocoFoundation${DEBUG_LIB_ENDING}.${LIB_ENDING}"
			"${POCO_LIB_DIR}/libPocoNetSSL${DEBUG_LIB_ENDING}.${LIB_ENDING}"
			"${POCO_LIB_DIR}/libPocoCrypto${DEBUG_LIB_ENDING}.${LIB_ENDING}"
			"pthread")
endif()


add_executable(trader_unit_tests ${TRADER_TEST_SOURCES})

if(WIN32)
	set_property(TARGET trader_unit_tests PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${BINARY_DEPEND_PATH}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(
	trader_unit_tests
	gtest
	gtest_main
	gmock
	${PTHREAD}
	trader
	database
	model
	features
	strategies
	stock_exchange
	Qt5::Core
	${POCO_LIBS}
	${SQLITE_LIB}
	${OPENSSL_LIBRARIES}
	${CURL_LIBRARIES})

if(WIN32)
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/trader_unit_tests PARENT_SCOPE)
else()
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/trader_unit_tests PARENT_SCOPE)
endif()
cmake_minimum_required(VERSION 3.0)

project(stock_exchange_unit_tests)

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

if(WIN32)
	set(POCO_LIBS        "${POCO_LIB_DIR}/PocoNet${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoUtil${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoXML${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoFoundation${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoJSON${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoNetSSL${DEBUG_LIB_ENDING}.${LIB_ENDING}"
		"${POCO_LIB_DIR}/PocoCrypto${DEBUG_LIB_ENDING}.${LIB_ENDING}")
endif()

file(GLOB TEST_SRC *.cpp)
file(GLOB TEST_INCLUDE_FILES *.h)

add_executable(stock_exchange_unit_tests ${TEST_INCLUDE_FILES} ${TEST_SRC})

if(WIN32)
	set_property(TARGET stock_exchange_unit_tests PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${BINARY_DEPEND_PATH}")
endif()

target_link_libraries(stock_exchange_unit_tests gtest gtest_main gmock ${PTHREAD} stock_exchange ${POCO_LIBS} Qt5::Core ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})

if(WIN32)
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/stock_exchange_unit_tests PARENT_SCOPE)
else()
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/stock_exchange_unit_tests PARENT_SCOPE)
endif()
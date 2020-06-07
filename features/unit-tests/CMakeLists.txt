cmake_minimum_required(VERSION 3.0)

project(features_unit_test)

file(GLOB FEATURE_TEST_SRC
        "*.h"
        "*.cpp"
        )

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

add_executable(features_unit_test ${FEATURE_TEST_SRC})

if(WIN32)
	set_property(TARGET features_unit_test PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${BINARY_DEPEND_PATH}")
endif()

target_link_libraries(features_unit_test gtest gtest_main gmock ${PTHREAD} features Qt5::Core Qt5::Network ${POCO_LIBS} ${OPENSSL_LIBRARIES})

if(WIN32)
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/features_unit_test PARENT_SCOPE)
else()
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/features_unit_test PARENT_SCOPE)
endif()

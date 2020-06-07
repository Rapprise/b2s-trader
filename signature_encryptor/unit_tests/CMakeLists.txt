cmake_minimum_required(VERSION 3.0)

project(signature_encryptor_unit_test)

file(GLOB SIGNATURE_ENCRYPTOR_SRC
		"*.h"
		"*.cpp"
	)

add_executable(signature_encryptor_unit_test ${SIGNATURE_ENCRYPTOR_SRC})

target_link_libraries(signature_encryptor_unit_test gtest gtest_main gmock ${PTHREAD} ${OPENSSL_LIBRARIES})

if(WIN32)	
	set_property(TARGET signature_encryptor_unit_test PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${BINARY_DEPEND_PATH}")
endif()

if(WIN32)
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/signature_encryptor_unit_test PARENT_SCOPE)
else()
	set(UNIT_TESTS ${UNIT_TESTS} ${CMAKE_CURRENT_BINARY_DIR}/signature_encryptor_unit_test PARENT_SCOPE)
endif()


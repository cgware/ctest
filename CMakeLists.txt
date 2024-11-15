cmake_minimum_required(VERSION 3.10)

project(ctest LANGUAGES C)

enable_testing()

find_program(LCOV lcov)
find_program(GENHTML genhtml)
add_custom_target(coverage
	COMMAND ${LCOV} -q -c -d ${CMAKE_BINARY_DIR} -o ${CMAKE_BINARY_DIR}/lcov.info
	COMMAND ${GENHTML} -q ${CMAKE_BINARY_DIR}/lcov.info -o ${CMAKE_SOURCE_DIR}/report/coverage
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(NOT MSVC)
	if(ARCH STREQUAL "x64")
		set(CMAKE_C_FLAGS "-m64")
	elseif(ARCH STREQUAL "x86")
		set(CMAKE_C_FLAGS "-m32")
	endif()
endif()

if(WIN32)
    set(LIB_EXT ".lib")
else()
	set(LIB_EXT ".a")
endif()

include(ExternalProject)
ExternalProject_Add(cbase_project
  GIT_REPOSITORY    https://github.com/cgware/cbase.git
  GIT_TAG           main
  SOURCE_DIR        "${CMAKE_SOURCE_DIR}/deps/cbase"
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  CMAKE_ARGS        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
  CMAKE_ARGS        "-DARCH=${ARCH}"
)

file(GLOB_RECURSE ctest_src src/*.h src/*.c)
add_library(ctest ${ctest_src})
add_dependencies(ctest cbase_project)
add_library(cbase STATIC IMPORTED)
set_target_properties(cbase PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/deps/cbase/bin/cbase/${ARCH}-${CMAKE_BUILD_TYPE}/cbase${LIB_EXT}
)
target_link_libraries(ctest PUBLIC cbase)
target_include_directories(ctest PUBLIC include ${CMAKE_SOURCE_DIR}/deps/cbase/include)
if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(ctest PRIVATE -Wall -Wextra -Werror -pedantic -coverage -O0 -ggdb)
		target_link_options(ctest PRIVATE -coverage)
	else()
		target_compile_options(ctest PRIVATE -Wall -Wextra -Werror -pedantic)
	endif()
endif()
set_target_properties(ctest PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/ctest/${ARCH}-Debug/
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/ctest/${ARCH}-Release/
	PREFIX ""
)

file(GLOB_RECURSE ctest_test_src test/*.h test/*.c)
add_executable(ctest_test ${ctest_test_src})
target_link_libraries(ctest_test ctest)
if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(ctest_test PRIVATE -Wall -Wextra -Werror -pedantic -coverage -O0 -ggdb)
		target_link_options(ctest_test PRIVATE -coverage)
	else()
		target_compile_options(ctest_test PRIVATE -Wall -Wextra -Werror -pedantic)
	endif()
endif()
set_target_properties(ctest_test PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/ctest_test/${ARCH}-Debug/
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/ctest_test/${ARCH}-Release/
)
add_test(ctest_build "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --config ${CMAKE_BUILD_TYPE} --target ctest_test)
add_test(ctest ${CMAKE_SOURCE_DIR}/bin/ctest_test/${ARCH}-${CMAKE_BUILD_TYPE}/ctest_test)
set_tests_properties(ctest PROPERTIES DEPENDS ctest_build)

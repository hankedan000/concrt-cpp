cmake_minimum_required (VERSION 3.1)

set(LIBNAME_ALL_UPPER "TEMPLATELIB")
# used for things like header install dir '/usr/lib/include/templatelib'
set(LIBNAME_ALL_LOWER "templatelib")

project(TemplateLibProject VERSION 1.0.0)

# ---------------------------------------------------------------------------------------
# Set TEMPLATELIB_MASTER_PROJECT to ON if we are building TemplateLib
# ---------------------------------------------------------------------------------------
# Check if TemplateLib is being used directly or via add_subdirectory, but allow overriding
if(NOT DEFINED TEMPLATELIB_MASTER_PROJECT)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(TEMPLATELIB_MASTER_PROJECT ON)
    else()
        set(TEMPLATELIB_MASTER_PROJECT OFF)
    endif()
endif()

option(TEMPLATELIB_BUILD_ALL "Build all artifacts" OFF)

# examples options
option(TEMPLATELIB_BUILD_EXAMPLES "Build examples" ${TEMPLATELIB_MASTER_PROJECT})

# testing options
option(TEMPLATELIB_BUILD_TESTS "Build tests" ${TEMPLATELIB_MASTER_PROJECT})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# build a multi-arch tuple for installing (ex. "x86_64-linux-gnu")
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)
string(TOLOWER ${CMAKE_CXX_COMPILER_ID} COMPILER_NAME_LOWER)
set(MULTI_ARCH_SUFFIX "${CMAKE_SYSTEM_PROCESSOR}-${SYSTEM_NAME_LOWER}-${COMPILER_NAME_LOWER}")

# add project's module directory to CMAKE_MODULE_PATH
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# update all submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	# Update submodules as needed
	option(GIT_SUBMODULE "${PROJECT_NAME} - Check submodules during build" ON)
	if(GIT_SUBMODULE)
		message(STATUS "Submodule update")
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						RESULT_VARIABLE GIT_SUBMOD_RESULT)
		if(NOT GIT_SUBMOD_RESULT EQUAL "0")
			message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
		endif()
	endif()
endif()

add_subdirectory(dependencies)
add_subdirectory(src)

# ---------------------------------------------------------------------------------------
# Build binaries
# ---------------------------------------------------------------------------------------
if (TEMPLATELIB_BUILD_EXAMPLES OR TEMPLATELIB_BUILD_ALL)
	message(STATUS "${PROJECT_NAME} - Generating examples")
	add_subdirectory(examples)
endif()

if (TEMPLATELIB_BUILD_TESTS OR TEMPLATELIB_BUILD_ALL)
	message(STATUS "${PROJECT_NAME} - Generating tests")
	include(CTest)
	add_subdirectory(test)
endif()
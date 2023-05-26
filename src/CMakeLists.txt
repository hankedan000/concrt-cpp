add_library(${LIBNAME_ALL_LOWER} ResourcePool.cpp)

# add namespaced alias so that library target is referencable similar
# to how it would when included from exports. this enabled project
# users to do add_sub_directory() on this project as submodule
add_library(${PROJECT_NAME}::${LIBNAME_ALL_LOWER} ALIAS ${LIBNAME_ALL_LOWER})

set(LIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/${LIBNAME_ALL_LOWER}")

target_include_directories(${LIBNAME_ALL_LOWER}
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include")

# generate the version header
configure_file("${LIB_INCLUDE_DIR}/version.h.in" "include/${LIBNAME_ALL_LOWER}/version.h")

# build a list of public header files to install
list(APPEND ${LIBNAME_ALL_UPPER}_PUBLIC_HEADERS
	"${LIB_INCLUDE_DIR}/ResourcePool.h"
	"${CMAKE_CURRENT_BINARY_DIR}/include/${LIBNAME_ALL_LOWER}/version.h"
)
set_target_properties(${LIBNAME_ALL_LOWER} PROPERTIES
	PUBLIC_HEADER "${${LIBNAME_ALL_UPPER}_PUBLIC_HEADERS}"
)
install(
	TARGETS ${LIBNAME_ALL_LOWER}
	EXPORT ${LIBNAME_ALL_LOWER}Targets
	ARCHIVE
		DESTINATION "lib/${MULTI_ARCH_SUFFIX}/${LIBNAME_ALL_LOWER}"
	LIBRARY
		DESTINATION "lib/${MULTI_ARCH_SUFFIX}/${LIBNAME_ALL_LOWER}"
	PUBLIC_HEADER
		DESTINATION "include/${LIBNAME_ALL_LOWER}"
)

# so this target can be used
export(EXPORT ${LIBNAME_ALL_LOWER}Targets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${LIBNAME_ALL_LOWER}Targets.cmake"
	NAMESPACE ${PROJECT_NAME}::
)
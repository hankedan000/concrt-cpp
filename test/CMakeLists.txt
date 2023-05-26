find_package(cppunit REQUIRED)

# ResourcePool unit test
add_executable(ResourcePoolTest ResourcePoolTest.cpp)
add_test(NAME ResourcePoolTest COMMAND ResourcePoolTest)

target_link_libraries(
	ResourcePoolTest
		${LIBNAME_ALL_LOWER}
		${CPPUNIT_LIBRARIES})

# ResourcePool throughput test app
add_executable(ResourcePoolThroughputTest ResourcePoolThroughputTest.cpp)
target_link_libraries(ResourcePoolThroughputTest ${LIBNAME_ALL_LOWER} pthread)
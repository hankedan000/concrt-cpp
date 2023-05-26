find_package(cppunit REQUIRED)

add_executable(MathTest MathTest.cpp)
add_test(NAME MathTest COMMAND MathTest)

target_link_libraries(
	MathTest
		Math
		${CPPUNIT_LIBRARIES})

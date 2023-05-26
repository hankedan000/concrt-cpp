#include "MathTest.h"

#include "templatelib/Math.h"

#include <sys/stat.h>
#include <unistd.h>

namespace MyLib
{

	MathTest::MathTest()
	{
	}

	void
	MathTest::setUp()
	{
		// run before each test case
	}

	void
	MathTest::tearDown()
	{
		// run after each test case
	}

	void
	MathTest::simple_tests()
	{
		CPPUNIT_ASSERT_EQUAL(4, add(2,2));
		CPPUNIT_ASSERT_EQUAL(1, sub(3,2));
		CPPUNIT_ASSERT_EQUAL(9, mult(3,3));
		CPPUNIT_ASSERT_EQUAL(3, div(9,3));
	}

}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(MyLib::MathTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}

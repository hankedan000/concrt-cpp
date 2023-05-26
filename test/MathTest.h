#pragma once

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace MyLib
{

	class MathTest : public CppUnit::TestFixture
	{
		CPPUNIT_TEST_SUITE(MathTest);
		CPPUNIT_TEST(simple_tests);
		CPPUNIT_TEST_SUITE_END();

	public:
		MathTest();
		void setUp();
		void tearDown();

	protected:
		void simple_tests();

	private:
		const std::string TEST_TMP_PATH = "test_tmp";

	};

}

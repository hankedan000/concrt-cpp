#pragma once

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class ResourcePoolTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(ResourcePoolTest);
	CPPUNIT_TEST(testInvalidSizes);
	CPPUNIT_TEST(testFullCycle);
	CPPUNIT_TEST_SUITE_END();

public:
	ResourcePoolTest();
	void setUp();
	void tearDown();

protected:
	void testInvalidSizes();
	void testFullCycle();

private:

};

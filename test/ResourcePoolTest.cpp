#include "ResourcePoolTest.h"

#include "concrt/ResourcePool.h"

#include <sys/stat.h>
#include <unistd.h>

struct TestResource
{
	std::string my_str;
	unsigned int my_int;
};

using TestPool = concrt::ResourcePool<TestResource,concrt::PC_Model::SPSC>;

ResourcePoolTest::ResourcePoolTest()
{
}

void
ResourcePoolTest::setUp()
{
	// run before each test case
}

void
ResourcePoolTest::tearDown()
{
	// run after each test case
}

void
ResourcePoolTest::testInvalidSizes()
{
	// make sure invalid sizes are handled correctly
	CPPUNIT_ASSERT_THROW(TestPool(0), std::runtime_error);
	CPPUNIT_ASSERT_THROW(TestPool(1), std::runtime_error);
	CPPUNIT_ASSERT_THROW(TestPool(2), std::runtime_error);
	CPPUNIT_ASSERT_THROW(TestPool(3), std::runtime_error);
	CPPUNIT_ASSERT_THROW(TestPool(7), std::runtime_error);
}

void
ResourcePoolTest::testFullCycle()
{
	const unsigned int SIZE = 8;
	const unsigned int CAPACITY = (SIZE - 1);
	TestPool pool(SIZE);

	CPPUNIT_ASSERT_EQUAL(CAPACITY, pool.capacity());
	CPPUNIT_ASSERT_EQUAL(CAPACITY, pool.available());

	for (unsigned int i=0; i<CAPACITY; i++)
	{
		TestResource *res;
		CPPUNIT_ASSERT_EQUAL(0,pool.acquire_busy(res));
		CPPUNIT_ASSERT(res != nullptr);
		CPPUNIT_ASSERT_EQUAL(CAPACITY - i - 1, pool.available());
		res->my_str = "helloworld";
		res->my_int = i;
		CPPUNIT_ASSERT_EQUAL(0,pool.produce(res));
	}

	for (unsigned int i=0; i<CAPACITY; i++)
	{
		TestResource *res;
		CPPUNIT_ASSERT_EQUAL(0,pool.consume_busy(res));
		CPPUNIT_ASSERT_EQUAL(i, pool.available());
		CPPUNIT_ASSERT(res != nullptr);
		CPPUNIT_ASSERT_EQUAL(i,res->my_int);
		CPPUNIT_ASSERT_EQUAL(0,pool.release(res));

		// make sure released resource is now available
		CPPUNIT_ASSERT_EQUAL(i+1, pool.available());
	}
}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(ResourcePoolTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}

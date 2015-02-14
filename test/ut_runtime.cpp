#include "unittest.h"

#include "../src/runtime/runtime.h"

void diag_memmgr(foliage::runtime::object* runtime_obj);

TEST(Runtime, Basic)
{
	using namespace foliage::runtime;
	using namespace foliage::utils;

	auto _runtime = runtime::create();
	ASSERT_NE(nullptr, _runtime) << "failed to create foliage runtime.";


}

static foliage::utils::byte_t global_buffer[16 * 1024L * 1024L];

TEST(Runtime, BasicEmbedded)
{
	using namespace foliage::runtime;
	using namespace foliage::utils;
	
	const size_t _memalign = 1024L;
	const size_t _memsize = 8 * 1024L;

	auto _runtime = runtime::create(global_buffer, _memsize);
	ASSERT_NE(nullptr, _runtime) << "failed to create foliage runtime.";

	diag_memmgr(_runtime.get());

}

class EmbeddedRuntimeTest : public ::testing::Test
{
public:
	std::unique_ptr<foliage::runtime::runtime, foliage::utils::placement_delete<foliage::runtime::runtime> > _runtime;
public:
	enum 
	{
		test_cases_count = 100000,
	};
	size_t test_cases[test_cases_count];
public:
	virtual void SetUp()
	{
		const size_t _memalign = 1024L;
		const size_t _memsize = 4 * 1024L;

		for (size_t i = 0; i < test_cases_count; i++)
			test_cases[i] = rand() % 10 + 1;
		_runtime = foliage::runtime::runtime::create(global_buffer, _memsize);
	}
};

TEST_F(EmbeddedRuntimeTest, AllocTest)
{
	auto allocator = _runtime->get_allocator < std::uintptr_t >();
	for (size_t i = 0; i < test_cases_count; i++)
	{
		//printf("%d\n", test_cases[i]);
		allocator.allocate(test_cases[i]);
	}
}

TEST_F(EmbeddedRuntimeTest, SystemAllocTest)
{
	auto allocator = std::allocator<std::uintptr_t>();
	for (size_t i = 0; i < test_cases_count; i++)
	{
		malloc(test_cases[i] << 2);
		//allocator.allocate(test_cases[i]);
	}
}

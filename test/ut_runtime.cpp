#include "unittest.h"

#include "../src/runtime/runtime.h"

TEST(Runtime, Basic)
{
	using namespace foliage::runtime;
	using namespace foliage::utils;

	auto _runtime = runtime::create();

}

TEST(Runtime, BasicEmbedded)
{
	using namespace foliage::runtime;
	using namespace foliage::utils;

	const size_t _memsize = 8 * 1024L;
	std::unique_ptr<byte_t[]> _memblock = std::make_unique<byte_t[]>(_memsize);

	auto _runtime = runtime::create(std::move(_memblock), _memsize);

}

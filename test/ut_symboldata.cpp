#include "unittest.h"

#include "../src/symboldata/symboldata.h"

TEST(SymbolData, Basic)
{
	using namespace foliage::symboldata;
	symboldatachain<> chain;
	symboldataref_t	s1 = chain.intern("at");
	symboldataref_t	s2 = chain.intern("cat");
	symboldataref_t	s3 = chain.intern("bat");
	EXPECT_EQ(0, s1);
	EXPECT_EQ(1, s2);
	EXPECT_EQ(3, s3);
}
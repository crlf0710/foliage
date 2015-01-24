#include "unittest.h"

namespace foliage_testing
{
	void process_params(int* pargc, char* argv[])
	{
		for (int i = 0; i < *pargc; i++)
		{
			if (std::strcmp(argv[i], "--pause-at-exit") == 0)
			{
				std::atexit([]()
				{
#ifdef GTEST_OS_WINDOWS
					system("pause");
#else
					system("read -p \"Press Enter key to continue...\"");
#endif
				});
			}
		}
	}
}

int main(int argc, char* argv[])
{
    std::setlocale(LC_ALL, "");
    ::testing::InitGoogleTest(&argc, argv);
	foliage_testing::process_params(&argc, argv);
    return RUN_ALL_TESTS();
}

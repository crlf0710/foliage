#include <gtest/gtest.h>
#include <clocale>

int main(int argc, char* argv[])
{
    std::setlocale(LC_ALL, "");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include <unittest++/UnitTest++.h>

TEST(WillFail) {
    CHECK(false);
}

int main()
{
    return UnitTest::RunAllTests();
}

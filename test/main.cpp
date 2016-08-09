#include <gtest/gtest.h>
#include <jx/Util.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    util::onStart(argc, argv);
    return RUN_ALL_TESTS();
}
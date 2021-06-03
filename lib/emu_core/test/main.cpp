#include "gtest/gtest.h"

int main(int argc, char **argv) {
    srand(static_cast<unsigned>(time(nullptr)));
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

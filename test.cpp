#include <gtest/gtest.h>
#include "controller.h"

TEST(controller_ctor, should_no_throw)
{
    ASSERT_NO_THROW(
        controller c;
    );
}
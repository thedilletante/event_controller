#include <gtest/gtest.h>
#include "controller.h"

TEST(controller_ctor, should_no_throw)
{
    ASSERT_NO_THROW(
        controller c;
    );
}

class Controller_subscribeTest
        : public ::testing::Test
{
protected:
    controller controller_;
};

void handler(int num)
{}

TEST_F(Controller_subscribeTest,
       should_not_throw)
{
    ASSERT_NO_THROW(
        controller_.subscribe<int>(handler);
    );
}

TEST(Controller_emplace, should_not_throw)
{
    controller cont;
    ASSERT_NO_THROW(cont.emplace<int>(2));
}
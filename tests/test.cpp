#include <gtest/gtest.h>
#include "controller.h"

using ctrl = controller::controller<>;

TEST(controller_ctor, should_no_throw)
{
    ASSERT_NO_THROW(
            ctrl c;
    );
}

class Controller_subscribeTest
        : public ::testing::Test
{
protected:
    ctrl controller_;
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
    ctrl cont;
    ASSERT_NO_THROW(cont.emplace(2));
}

TEST(Controller_delivery, should_just_work)
{
    ctrl cont;
    int num = 0;
    auto task = [&num](int evt){
        num = evt;
    };

    cont.subscribe<int>(task);

    cont.emplace(4);
    cont.do_delivery();
    ASSERT_EQ(4, num);


    auto task1 = [](const int& a){
        std::cout << "Hello" << a << std::endl;
    };
    cont.subscribe<int>(task1);
    cont.subscribe<int&>(task1);
    cont.subscribe<const int&>(task1);
    cont.emplace(2);
    cont.do_delivery();

    int a = 1;
    cont.emplace(a);
    cont.do_delivery();

    a = 3;
    cont.emplace<int&>(a);
    cont.do_delivery();

    a = 4;
    cont.emplace<const int&>(a);
    cont.do_delivery();


    class a_class{};
    auto task2 = [](const a_class& ca){
        std::cout << "Hello with a" << std::endl;
    };

   // cont.subscribe

}
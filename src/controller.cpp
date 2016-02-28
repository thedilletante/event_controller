#include "controller.h"
#include <iostream>

controller::~controller()
{
    notify_subscription_thread(subscription_message_type::STOP);
    subscription_thread_.join();
}

controller::event_holder::~event_holder() {}
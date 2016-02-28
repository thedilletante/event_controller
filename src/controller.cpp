#include "controller.h"
#include <iostream>

controller::~controller()
{
    notify_subscription_thread(subscription_message_type::STOP);
    subscription_thread_.join();
}

void controller::do_delivery()
{
    for (auto& event_holder : events_)
    {
        event_holder->process();
    }
    events_.clear();
}

controller::event_holder::~event_holder() {}
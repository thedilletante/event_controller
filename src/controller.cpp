#include "controller.h"

controller::~controller()
{
    notify_subscription_thread(subscription_message_type::STOP);
    subscription_thread_.join();
}

void controller::do_delivery()
{
    for (auto& event_task : events_)
    {
        event_task();
    }
    events_.clear();
}
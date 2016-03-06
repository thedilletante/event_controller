#ifndef EVENT_CONTROLLER_CONTROLLER_H
#define EVENT_CONTROLLER_CONTROLLER_H

#include <functional>
#include <list>
#include <memory>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "controller_types.h"
#include "thread_local_subscriber_storage.h"

namespace controller
{


template <class SUBSCRIPTION_STORAGE = thread_local_subscriber_storage>
class controller
{
    template <class EVENT>
    using subscription_id = typename thread_local_subscriber_storage::subscription_id<EVENT>;

public:
    template <class EVENT>
    subscription_id<EVENT> subscribe(handler_t<EVENT> handler)
    {
        return storage_.push_handler(handler);
    }

    template <class EVENT>
    bool emplace(EVENT&& event)
    {
        auto event_process_task = [this](invoke_type_t<EVENT> event){
            storage_.delivery_event(std::forward<EVENT>(event));
        };
        events_.push_back(std::bind(event_process_task, std::forward<EVENT>(event)));
        return true;
    }

    void do_delivery()
    {
        for (auto& event_task : events_)
        {
            event_task();
        }
        events_.clear();
    }

private:
    std::list<std::function<void()>> events_;
    SUBSCRIPTION_STORAGE storage_;
};

}


#endif // EVENT_CONTROLLER_CONTROLLER_H

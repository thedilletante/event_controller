#ifndef EVENT_CONTROLLER_THREAD_LOCAL_SUBSCRIBER_STORAGE_H
#define EVENT_CONTROLLER_THREAD_LOCAL_SUBSCRIBER_STORAGE_H

#include <list>
#include <future>
#include <functional>
#include "controller_types.h"

namespace controller
{

class thread_local_subscriber_storage
{
private:

    template <class EVENT>
    class subscriber_holder
    {
    public:
        explicit subscriber_holder(handler_t<EVENT> handler)
                : handler_(handler) {}
        handler_t<EVENT> handler_;
    };

    template <class EVENT>
    using subscriber_holder_list = std::list<subscriber_holder<EVENT>>;

public:
    template <class EVENT>
    using subscription_id = typename subscriber_holder_list<EVENT>::const_iterator;


    thread_local_subscriber_storage(const thread_local_subscriber_storage &) = delete;
    thread_local_subscriber_storage &
            operator=(const thread_local_subscriber_storage &) = delete;

    explicit thread_local_subscriber_storage()
            : subscription_thread_{&thread_local_subscriber_storage::initialize_subscription_thread, this}
    {
        make_sure_thread_started();
    }

    ~thread_local_subscriber_storage()
    {
        stop_subsription_thread();
    }


    template <class EVENT>
    subscription_id<EVENT> push_handler(handler_t<EVENT> handler)
    {
        std::promise<subscription_id<EVENT>> pushed;
        auto pushed_task = [this, &pushed, &handler]() {
            auto &subscribers = get_subscribers<EVENT>();
            subscribers.push_back(subscriber_holder<EVENT>{handler});
            pushed.set_value(--subscribers.end());
        };

        {
            std::lock_guard<std::mutex> lock(subscription_mutex_);
            pushing_tasks_.emplace(pushed_task);
        }

        notify_subscription_thread(subscription_message_type::PUSH_HANDLER);
        auto future = pushed.get_future();

        future.wait();
        return future.get();
    }

    template <class EVENT>
    void delivery_event(EVENT&& event)
    {
        // request for subscribers
        std::promise<subscriber_holder_list<EVENT>> fetched;
        auto fetched_taks = [this, &fetched]() {
            fetched.set_value(get_subscribers<EVENT>());
        };

        {
            std::lock_guard<std::mutex> lock(subscription_mutex_);
            fetching_tasks_.emplace(fetched_taks);
        }
        notify_subscription_thread(subscription_message_type::GET_HANDLER);
        auto future = fetched.get_future();
        future.wait();
        auto subscribers = future.get();

        // and call to handlers
        for (auto& subscriber : subscribers)
        {
            subscriber.handler_(std::forward<EVENT>(event));
        }
    }

    template <class EVENT>
    void remove_handler(subscription_id<EVENT> id);

private:

    template <class EVENT>
    subscriber_holder_list<EVENT>& get_subscribers()
    {
        static thread_local subscriber_holder_list<EVENT> subscribers;
        return subscribers;
    }

    std::thread subscription_thread_;
    std::promise<void> subscription_ready_;

    enum class subscription_message_type : uint8_t
    {
        IDLE,
        PUSH_HANDLER,
        GET_HANDLER,
        REMOVE_HANDLER,
        STOP
    };
    std::condition_variable subscription_cv_;
    std::mutex subscription_mutex_;
    subscription_message_type subscription_message_ = subscription_message_type::IDLE;

    using subscription_task_t = std::function<void()>;
    using subscription_task_queue_t = std::queue<subscription_task_t>;

    subscription_task_queue_t pushing_tasks_;
    subscription_task_queue_t fetching_tasks_;
    subscription_task_queue_t removing_tasks_;
    void make_sure_thread_started()
    {
        subscription_ready_.get_future().wait();
    }

    void stop_subsription_thread()
    {
        notify_subscription_thread(subscription_message_type::STOP);
        subscription_thread_.join();
    }

    void initialize_subscription_thread()
    {
        std::unique_lock<std::mutex> lock(subscription_mutex_);
        subscription_ready_.set_value();

        do
        {
            subscription_cv_.wait(lock, [this](){
                return subscription_message_type::IDLE != subscription_message_;
            });

            switch (subscription_message_)
            {
                case subscription_message_type::STOP:
                    return;

                case subscription_message_type::PUSH_HANDLER:
                    // handle push
                    if (!pushing_tasks_.empty())
                    {
                        pushing_tasks_.front()();
                        pushing_tasks_.pop();
                    }
                    break;
                case subscription_message_type::GET_HANDLER:
                    // handle fetching
                    if (!fetching_tasks_.empty())
                    {
                        fetching_tasks_.front()();
                        fetching_tasks_.pop();
                    }
                    break;
                case subscription_message_type::REMOVE_HANDLER:
                    // handle removing
                    if (!removing_tasks_.empty())
                    {
                        removing_tasks_.front()();
                        removing_tasks_.pop();
                    }
                    break;
                default:
                    // something wrong
                    break;
            }

            // we are ready to handle new requests
            subscription_message_ = subscription_message_type::IDLE;
        }
        while (true);
    }

    void notify_subscription_thread(subscription_message_type message)
    {
        std::lock_guard<std::mutex> lock(subscription_mutex_);
        subscription_message_ = message;
        subscription_cv_.notify_one();
    }
};

}


#endif //EVENT_CONTROLLER_THREAD_LOCAL_SUBSCRIBER_STORAGE_H

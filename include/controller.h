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

class controller
{
public:
    template <class EVENT>
    using handler_t = std::function<void(EVENT)>;
private:
    class event_holder;
    using event_holder_ptr = std::unique_ptr<event_holder>;
    using event_holder_list = std::list<event_holder_ptr>;

    template <class EVENT>
    class subscriber_holder
    {
    public:
        explicit subscriber_holder(handler_t<EVENT> handler)
                : handler_(handler) {}
    private:
        handler_t<EVENT> handler_;
    };

    template <class EVENT>
    using subscriber_holder_list = std::list<subscriber_holder<EVENT>>;

    template <class EVENT>
    using subscription_id = typename subscriber_holder_list<EVENT>::const_iterator;

public:
    explicit controller()
        : subscription_thread_{&controller::initialize_subscription_thread, this}
    {
        subscription_ready_.get_future().wait();
    }
    ~controller();

    template <class EVENT>
    subscription_id<EVENT> subscribe(handler_t<EVENT> handler);

    template <class EVENT, typename... Args>
    bool emplace(Args&& ...args);

private:
    event_holder_list events_;

    template <class EVENT>
    subscriber_holder_list<EVENT>& get_subscribers()
    {
        static thread_local subscriber_holder_list<EVENT> subscribers;
        return subscribers;
    }

    std::thread subscription_thread_;
    std::promise<void> subscription_ready_;
    std::promise<void> subscription_done_;

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

template <class EVENT>
controller::subscription_id<EVENT> controller::subscribe(handler_t<EVENT> handler)
{
    std::promise<subscription_id<EVENT>> pushed;
    auto pushed_task = [this, &pushed, &handler]() {
        auto &subscribers = get_subscribers<EVENT>();
        subscribers.push_back(subscriber_holder < EVENT > {handler});
        pushed.set_value(--subscribers.end());
    };

    {
        std::lock_guard<std::mutex> lock(subscription_mutex_);
        pushing_tasks_.emplace(pushed_task);
    }

    notify_subscription_thread(subscription_message_type::PUSH_HANDLER);
    pushed.get_future().wait();

    return pushed.get_future().get();
}


class controller::event_holder
{ public: virtual ~event_holder(); };


template <class EVENT, typename... Args>
bool controller::emplace(Args&& ...args)
{
    class concrete_event_holder : public controller::event_holder
    {
    public:
        explicit concrete_event_holder(Args&& ...args)
                : event_(std::forward<Args>(args)...) {}
    private:
        EVENT event_;
    };
    // get current subscribers for this event
    // create event holder with subscribers


    //events_.emplace_back(new concrete_event_holder<EVENT>(std::forward<Args>(args)...));
    return true;
}

#endif // EVENT_CONTROLLER_CONTROLLER_H

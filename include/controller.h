#ifndef EVENT_CONTROLLER_CONTROLLER_H
#define EVENT_CONTROLLER_CONTROLLER_H

#include <functional>
#include <list>
#include <memory>

class controller
{
private:
    class event_holder;
    using event_holder_ptr = std::unique_ptr<event_holder>;
    using event_holder_list = std::list<event_holder_ptr>;

    class subscriber_holder;
    using subscriber_holder_ptr = std::unique_ptr<subscriber_holder>;
    using subscriber_holder_list = std::list<subscriber_holder_ptr>;
    using subscribtion_id = subscriber_holder_list::const_iterator;

public:
    template <class EVENT>
    using handler_t = std::function<void(EVENT)>;

    ~controller();

    template <class EVENT>
    subscribtion_id subscribe(handler_t<EVENT> handler);

    template <class EVENT, typename... Args>
    bool emplace(Args&& ...args);

private:
    event_holder_list events_;
    subscriber_holder_list subscribers_;
};

class controller::event_holder
{
public:
    virtual ~event_holder();
};

class controller::subscriber_holder
{
public:
    virtual ~subscriber_holder();
};

template <class EVENT>
controller::subscribtion_id controller::subscribe(handler_t<EVENT> handler)
{
    using concrete_handler_t = handler_t<EVENT>;
    class concrete_subscriber_holder : public subscriber_holder
    {
    public:
        explicit concrete_subscriber_holder(concrete_handler_t concrete_handler)
            : handler_(concrete_handler) { }

        void handle(EVENT&& event)
        {
            handler_(std::forward<EVENT>(event));
        }
    private:
        concrete_handler_t handler_;
    };

    subscribers_.emplace_back(new concrete_subscriber_holder(handler));
    return --subscribers_.end();
}

template <class EVENT, typename... Args>
bool controller::emplace(Args&& ...args)
{
    class concrete_event_holder : public event_holder
    {
    public:
        explicit concrete_event_holder(Args&& ...args)
        : event_(std::forward<Args>(args)...)
        {}

        EVENT event_;
    };

    events_.emplace_back(new concrete_event_holder(std::forward<Args>(args)...));
    return true;
}

#endif // EVENT_CONTROLLER_CONTROLLER_H

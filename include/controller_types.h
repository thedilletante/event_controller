#ifndef EVENT_CONTROLLER_CONTROLLER_TYPES_H
#define EVENT_CONTROLLER_CONTROLLER_TYPES_H

namespace controller
{

template <class EVENT>
using handler_t = std::function<void(EVENT)>;


template <typename Arg>
struct invoke_type
        : std::add_lvalue_reference<Arg> { };

template <typename T>
struct invoke_type<std::reference_wrapper<T>> {
    using type = T&;
};

template <typename T>
using invoke_type_t = typename invoke_type<T>::type;

}


#endif //EVENT_CONTROLLER_CONTROLLER_TYPES_H

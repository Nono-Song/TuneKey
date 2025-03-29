//
// Created by Huanming Song on 3/12/25.
//

#pragma once

#include "Button.h"
#include <any>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <fmt/format.h>


template <typename T>
class EventQueue
{
public:
    using identifier_type = typename T::identifier_type;
    using action_type = typename T::action_type;

    struct Event
    {
        Event(identifier_type id, action_type action, std::any params) : uuid(id), action(action),
                                                                         params(std::move(params))
        {
        }

        const std::tuple<>& get_content() const
        {
            return {uuid, action, params};
        }

        const identifier_type uuid;
        const action_type action;
        const std::any params;
    };

    void Push(Event&& evt)
    {
        std::unique_lock lock(mutex_);
        while (queue_.size() > max_size)
        {
            notfull_.wait(lock);
        }

        queue_.push(std::move(evt));
        nonempty_.notify_all();
    }

    Event& Pop()
    {
        std::unique_lock lock(mutex_);
        while (queue_.empty())
        {
            nonempty_.wait(lock);
        }

        auto event = queue_.front();
        queue_.pop();
        notfull_.notify_all();
        return event;
    }

private:
    static constexpr std::size_t max_size = 128;
    std::queue<Event> queue_;
    std::mutex mutex_;
    std::condition_variable_any nonempty_;
    std::condition_variable_any notfull_;
};

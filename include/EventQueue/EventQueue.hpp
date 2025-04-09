//
// Created by Huanming Song on 3/12/25.
//

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <utility>

class QueueStoppedException final : public std::runtime_error
{
public:
    QueueStoppedException() : std::runtime_error("EventQueue operation aborted due to stop request")
    {
    }
};

template <typename Event>
class EventQueue
{
public:
    explicit EventQueue(const std::stop_token& stoken): stoken_(stoken)
    {
    }

    void push(Event&& evt)
    {
        std::unique_lock lock(mutex_);
        if (stoken_.stop_requested())
        {
            throw QueueStoppedException();
        }

        if (!notfull_.wait(lock, stoken_, [this]() { return queue_.size() < max_size; }))
        {
            throw QueueStoppedException();
        }

        queue_.emplace(std::forward<Event>(evt));
        nonempty_.notify_all();
    }

    Event pop()
    {
        std::unique_lock lock(mutex_);
        if (stoken_.stop_requested())
        {
            throw QueueStoppedException();
        }
        if (!nonempty_.wait(lock, stoken_, [this]() { return !queue_.empty(); }))
        {
            throw QueueStoppedException();
        }

        auto event = std::move(queue_.front());
        queue_.pop();
        lock.unlock();

        notfull_.notify_all();
        return event;
    }

private:
    static constexpr std::size_t max_size = 128;
    std::queue<Event> queue_;
    std::mutex mutex_;
    std::stop_token stoken_;
    std::condition_variable_any nonempty_;
    std::condition_variable_any notfull_;
};

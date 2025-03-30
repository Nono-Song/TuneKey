//
// Created by Huanming Song on 3/12/25.
//

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename Event>
class EventQueue
{
public:
    void push(Event&& evt)
    {
        std::unique_lock lock(mutex_);
        notfull_.wait(lock, [this]() { return queue_.size() <= max_size; });

        queue_.push(std::forward<Event>(evt));
        nonempty_.notify_all();
    }

    Event pop()
    {
        std::unique_lock lock(mutex_);
        nonempty_.wait(lock, [this]() { return !queue_.empty(); });

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
    std::condition_variable_any nonempty_;
    std::condition_variable_any notfull_;
};

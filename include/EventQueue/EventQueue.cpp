//
// Created by Huanming Song on 3/12/25.
//
#include "EventQueue.h"

void EventQueue::Push(Event&& evt)
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.size() > max_size)
    {
        notfull_.wait(lock);
    }

    queue_.push(std::move(evt));
    nonempty_.notify_all();
}

Event EventQueue::Pop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty())
    {
        nonempty_.wait(lock);
    }

    auto event = queue_.front();
    queue_.pop();
    notfull_.notify_all();
    return event;
}

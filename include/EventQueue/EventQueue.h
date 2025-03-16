//
// Created by Huanming Song on 3/12/25.
//

#pragma once

#include "Button.h"
#include <any>
#include <queue>
#include <mutex>
#include <condition_variable>

struct Event
{
    Button::uuid_t uuid; // Target button's uuid
    Button::ActionType action;
    std::any params;
};

class EventQueue
{
public:
    void Push(Event&& evt);

    Event Pop();

private:
    static constexpr std::size_t max_size = 128;
    std::queue<Event> queue_;
    std::mutex mutex_;
    std::condition_variable_any nonempty_;
    std::condition_variable_any notfull_;
};

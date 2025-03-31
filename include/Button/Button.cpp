//
// Created by Schizoneurax on 3/11/2025.
//
#include "Button.h"

#include <utility>
#include <EventQueue.hpp>

Button::~Button() noexcept
{
    release();
}

Button::Button(const name_type& name,
               EventQueue<ButtonEvent>* queue)
    : Button(name, "", queue)
{
};

Button::Button(name_type name,
               const std::string& path,
               EventQueue<ButtonEvent>* queue)
    : name_(std::move(name)),
      id_(next_id_++),
      file_path_(path),
      event_queue_(queue)
{
};

void Button::playAudio() const
{
    event_queue_->push({id_, ActionType::Play});
}

void Button::pauseAudio() const
{
    event_queue_->push({id_, ActionType::Pause});
}

void Button::resumeAudio() const
{
    event_queue_->push({id_, ActionType::Resume});
}

void Button::release() const
{
    event_queue_->push({id_, ActionType::Release});
}

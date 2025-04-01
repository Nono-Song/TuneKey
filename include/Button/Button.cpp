//
// Created by Schizoneurax on 3/11/2025.
//
#include "Button.h"

#include <utility>
#include <EventQueue.hpp>

Button::~Button() noexcept = default;

Button::Button(const name_type& name,
               EventQueue<event_type>* queue)
    : Button(name, "", queue)
{
};

Button::Button(name_type name,
               const std::string& path,
               EventQueue<event_type>* queue)
    : name_(std::move(name)),
      id_(next_id_++),
      file_path_(path),
      event_queue_(queue)
{
};

Button::Button(Button&& other) noexcept
    : name_{std::move(other.name_)},
      id_{other.id_},
      file_path_{std::move(other.file_path_)},
      event_queue_(other.event_queue_)
{
}

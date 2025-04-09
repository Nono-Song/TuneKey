//
// Created by Schizoneurax on 3/11/2025.
//
#include "Button.h"

#include <utility>
#include <EventQueue.hpp>

Button::~Button() noexcept = default;

Button::Button(const name_type& name,
               const identifier_type& id,
               std::unique_ptr<event_queue>& queue)
    : Button(name, id, "", queue)
{
};

Button::Button(name_type name,
               const identifier_type& id,
               filename_type path,
               std::unique_ptr<event_queue>& queue)
    : event_queue_(queue),
      name_(std::move(name)),
      id_(id),
      file_path_(std::move(path))
{
};

Button::Button(Button&& other) noexcept
    : event_queue_(other.event_queue_),
      name_{std::move(other.name_)},
      id_{other.id_},
      file_path_{std::move(other.file_path_)}
{
}

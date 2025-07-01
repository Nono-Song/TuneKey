//
// Created by Schizoneurax on 4/10/2025.
//
#include "Button.hpp"
#include <IAudioController.hpp>

using namespace TuneKey;

Button::Button(const name_type& name, const identifier_type id)
    : Button(name, id, "")
{
}

Button::Button(name_type name, const identifier_type id, filename_type path)
    : name_(std::move(name)), id_(id), file_path_(std::move(path))
{
}

Button::Button(Button&& other) noexcept
    : name_{std::move(other.name_)}, id_{other.id_},
      file_path_{std::move(other.file_path_)}
{
}

Button::~Button() noexcept = default;
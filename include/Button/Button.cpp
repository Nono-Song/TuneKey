//
// Created by Schizoneurax on 4/10/2025.
//
#include "Button.hpp"
#include <AudioController.h>

Button::Button(const name_type& name, const identifier_type id, AudioController* controller)
    : Button(name, id, "", controller)
{
}

Button::Button(name_type name, const identifier_type id, filename_type path, AudioController* controller)
    : controller_(controller), name_(std::move(name)), id_(id), file_path_(std::move(path))
{
}

Button::Button(Button&& other) noexcept
    : controller_(other.controller_), name_{std::move(other.name_)}, id_{other.id_},
      file_path_{std::move(other.file_path_)}
{
}

Button::~Button() noexcept = default;


template <ButtonEvent Evt>
void Button::handleEvent() const
{
    if constexpr (std::is_same_v<Evt, PlayEvent>)
    {
        controller_->play(file_path_);
    }
    else if constexpr (std::is_same_v<Evt, PauseEvent>)
    {
        controller_->pause();
    }
    else if constexpr (std::is_same_v<Evt, ResumeEvent>)
    {
        controller_->resume();
    }
    else if constexpr (std::is_same_v<Evt, StopEvent>)
    {
        controller_->stop();
    }
    else
    {
        static_assert(false,
                      "Unhandled case in Button::handleEvent: "
                      "not a Button event or change in ButtonEvent concept");
    }
}

template <ButtonAttr Attr>
Button::ProjVariant Button::Projector()
{
    using U = Attr;
    if constexpr (std::is_same_v<U, name_type>)
    {
        return createProjector<&Button::name_>();
    }
    else if constexpr (std::is_same_v<U, identifier_type>)
    {
        return createProjector<&Button::id_>();
    }
    else if constexpr (std::is_same_v<U, filename_type>)
    {
        return createProjector<&Button::file_path_>();
    }
    else
    {
        static_assert(false,
                      "Unhandled case in Button::Projector: Update if constexpr chain for ButtonAttr!");
        throw std::logic_error("Unhandled case in Button::Projector");
    }
}

template <>
void Button::handleEvent<PlayEvent>() const;

template <>
void Button::handleEvent<PauseEvent>() const;

template <>
void Button::handleEvent<ResumeEvent>() const;

template <>
void Button::handleEvent<StopEvent>() const;

template <>
Button::ProjVariant Button::Projector<identifier_type>();

template <>
Button::ProjVariant Button::Projector<name_type>();

template <>
Button::ProjVariant Button::Projector<filename_type>();

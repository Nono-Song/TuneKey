//
// Created by Schizoneurax on 4/10/2025.
//

#pragma once

#include "Button.hpp"

class PlayButton final : public Button
{
public:
    PlayButton(const name_type& name, const identifier_type id, AudioController* controller)
        : Button(name, id, "", controller)
    {
    }

    PlayButton(name_type name, const identifier_type id, filename_type path, AudioController* controller)
        : Button(std::move(name), id, std::move(path), controller)
    {
    }

    PlayButton(PlayButton&& other) noexcept
        : Button(std::move(other))
    {
    }

    void interact() const override { handleEvent<PlayEvent>(); };
};

class PauseButton final : public Button
{
public:
    PauseButton(const name_type& name, const identifier_type id, AudioController* controller)
        : Button(name, id, "", controller)
    {
    }

    PauseButton(PauseButton&& other) noexcept
        : Button(std::move(other))
    {
    }

    void interact() const override { handleEvent<PauseEvent>(); };
};

class ResumeButton final : public Button
{
public:
    ResumeButton(name_type name, const identifier_type id, AudioController* controller)
        : Button(std::move(name), id, "", controller)
    {
    }

    ResumeButton(ResumeButton&& other) noexcept
        : Button(std::move(other))
    {
    }

    void interact() const override { handleEvent<ResumeEvent>(); };
};

class StopButton final : public Button
{
public:
    StopButton(name_type name, const identifier_type id, AudioController* controller)
        : Button(std::move(name), id, "", controller)
    {
    }

    StopButton(StopButton&& other) noexcept : Button(std::move(other))
    {
    }

    void interact() const override { handleEvent<StopEvent>(); };
};

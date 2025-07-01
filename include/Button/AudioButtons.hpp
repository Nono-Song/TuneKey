//
// Created by Schizoneurax on 4/10/2025.
//

#pragma once

#include "Button.hpp"
using namespace TuneKey;
    class PlayButton final : public Button
    {
    public:
        PlayButton(const name_type& name, const identifier_type id)
            : Button(name, id, "")
        {
        }

        PlayButton(name_type name, const identifier_type id, filename_type path)
            : Button(std::move(name), id, std::move(path))
        {
        }

        PlayButton(PlayButton&& other) noexcept
            : Button(std::move(other))
        {
        }

        void interact(IAudioController*) const override;
    };

    class PauseButton final : public Button
    {
    public:
        PauseButton(const name_type& name, const identifier_type id)
            : Button(name, id, "")
        {
        }

        PauseButton(PauseButton&& other) noexcept
            : Button(std::move(other))
        {
        }

        void interact(IAudioController*) const override;
    };

    class ResumeButton final : public Button
    {
    public:
        ResumeButton(name_type name, const identifier_type id)
            : Button(std::move(name), id, "")
        {
        }

        ResumeButton(ResumeButton&& other) noexcept
            : Button(std::move(other))
        {
        }

        void interact(IAudioController*) const override;
    };

    class StopButton final : public Button
    {
    public:
        StopButton(name_type name, const identifier_type id)
            : Button(std::move(name), id, "")
        {
        }

        StopButton(StopButton&& other) noexcept : Button(std::move(other))
        {
        }

        void interact(IAudioController*) const override;
    };

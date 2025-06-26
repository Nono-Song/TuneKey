//
// Created by Schizoneurax on 3/18/2025.
//

#pragma once
#include <Event.hpp>

//@formatter:off
struct IAudioController
{
    virtual ~IAudioController() = default;
    virtual void start() = 0;
    virtual void shutdown() = 0;

    // Change state
    virtual void play(identifier_type, const filename_type&) = 0;
    virtual void pause(identifier_type) = 0;
    virtual void resume(identifier_type) = 0;
    virtual void stop(identifier_type) = 0;

    [[nodiscard]] virtual std::optional<identifier_type> active_button() const = 0;
};

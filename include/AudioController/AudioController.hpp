//
// Created by Schizoneurax on 3/18/2025.
//

#pragma once
#include <Event.hpp>
#include <memory>

//@formatter:off
struct AudioController
{
    virtual ~AudioController();

    virtual void start() = 0;
    virtual void shutdown() = 0;

    // Change state
    virtual void play(identifier_type, const boost::filesystem::path&) = 0;
    virtual void pause(identifier_type) = 0;
    virtual void resume(identifier_type) = 0;
    virtual void stop(identifier_type) = 0;

    [[nodiscard]] virtual std::optional<identifier_type> active_button() const = 0;

    static std::unique_ptr<AudioController> create();
};

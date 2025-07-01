//
// Created by Schizoneurax on 4/6/2025.
//

#pragma once

#include <optional>
#include <Typedefs.hpp>

namespace TuneKey
{
    //@formatter:off
    struct PlayEvent
    {
        explicit PlayEvent(const identifier_type id, filename_type path)
        : id(id), filename(std::move(path)) {}

        const identifier_type id;
        const filename_type filename;
    };

    struct PauseEvent
    {
        explicit PauseEvent(const identifier_type id): id(id) {}
        const identifier_type id{};
    };

    struct ResumeEvent
    {
        explicit ResumeEvent(const identifier_type id): id(id) {}
        const identifier_type id;
    };

    struct StopEvent
    {
        explicit StopEvent(const identifier_type id): id(id) {}
        const identifier_type id;
    };

    struct AudioReadyEvent {};

    struct AudioFinishedEvent
    {
        explicit AudioFinishedEvent(const identifier_type id): id(id) {}
        const identifier_type id;
    };

    struct AudioErrorEvent
    {
        AudioErrorEvent(const std::optional<identifier_type>& id, std::string s)
        : id(id), error_msg(std::move(s)) {}
        const std::optional<identifier_type> id;
        const std::string error_msg;
    };

    struct ShutdownEvent {};
}
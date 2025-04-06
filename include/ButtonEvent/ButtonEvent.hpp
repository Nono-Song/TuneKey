//
// Created by Schizoneurax on 4/6/2025.
//

#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <boost/filesystem.hpp>

using identifier_type = uint64_t;
using name_type = std::string;
using filename_type = boost::filesystem::path;

struct PlayEvent
{
    explicit PlayEvent(identifier_type id, boost::filesystem::path path):
        id(id),
        filename(std::move(path))
    {
    }

    identifier_type id;
    filename_type filename;
};

struct PauseEvent
{
    explicit PauseEvent(const identifier_type id): id(id) {}
    identifier_type id{};
};

struct ResumeEvent
{
    explicit ResumeEvent(const identifier_type id): id(id) {}
    identifier_type id;
};

struct StopEvent
{
    explicit StopEvent(const identifier_type id): id(id) {}
    identifier_type id;
};

struct ShutdownEvent {};

using ButtonEvent = std::variant<PlayEvent,
                                 PauseEvent, ResumeEvent,
                                 StopEvent, ShutdownEvent>;
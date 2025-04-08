//
// Created by Schizoneurax on 4/6/2025.
//

#pragma once

#include <cstdint>
#include <string>
#include <boost/filesystem.hpp>

using identifier_type = uint64_t;
using name_type = std::string;
using filename_type = boost::filesystem::path;

//@formatter:off
struct PlayEvent
{
    explicit PlayEvent(const identifier_type id, boost::filesystem::path path)
    : id(id), filename(std::move(path)) {}

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

struct AudioFinishedEvent
{
    explicit AudioFinishedEvent(const identifier_type id): id(id) {}
    identifier_type id;
};

struct AudioErrorEvent
{
    explicit AudioErrorEvent(const identifier_type id, std::string s)
    : id(id), error_msg(std::move(s)) {}
    identifier_type id;
    std::string error_msg;
};

struct ShutdownEvent {};

//
// Created by Huanming Song on 6/26/25.
//

#pragma once
#include <cstdint>
#include <string>
#include <filesystem>

namespace TuneKey
{
    using identifier_type = uint64_t;
    using name_type = std::string;
    using filename_type = std::filesystem::path;
    using timestamp_type = std::chrono::system_clock::time_point;
}

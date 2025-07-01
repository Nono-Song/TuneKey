//
// Created by Huanming Song on 4/16/25.
//

#pragma once
#include <memory>

#include <IAudioController.hpp>
namespace TuneKey {
    class AudioControllerFactory
    {
        public:
        static std::unique_ptr<IAudioController> createAudioController();
    };
}
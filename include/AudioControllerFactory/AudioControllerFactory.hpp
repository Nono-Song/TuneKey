//
// Created by Huanming Song on 4/16/25.
//

#pragma once
#include <memory>

#include <IAudioController.hpp>
class AudioControllerFactory {
  public:
    static std::unique_ptr<IAudioController> createAudioController();
};
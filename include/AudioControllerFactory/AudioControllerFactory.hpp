//
// Created by Huanming Song on 4/16/25.
//

#pragma once
#include <memory>

#include <AudioController.hpp>
class AudioControllerFactory {
  public:
    static std::unique_ptr<AudioController> createAudioController();
};
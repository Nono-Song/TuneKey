//
// Created by Huanming Song on 4/16/25.
//

#include "AudioControllerFactory.hpp"
#include <AudioControllerImpl.hpp>

std::unique_ptr<IAudioController> AudioControllerFactory::createAudioController()
{
  return std::make_unique<AudioControllerImpl>();
}
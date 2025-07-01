//
// Created by Huanming Song on 4/16/25.
//

#include "AudioControllerFactory.hpp"
#include <AudioControllerImpl.hpp>

using namespace TuneKey;
std::unique_ptr<IAudioController> AudioControllerFactory::createAudioController()
{
  return std::make_unique<AudioControllerImpl>();
}
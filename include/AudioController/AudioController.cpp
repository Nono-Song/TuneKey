//
// Created by Schizoneurax on 3/18/2025.
//

#include "AudioController.hpp"
#include "AudioControllerImpl.hpp"
#include <EventQueue.hpp>

std::unique_ptr<AudioController> AudioController::create()
{
    return std::make_unique<AudioControllerImpl>();
}

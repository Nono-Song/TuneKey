//
// Created by Huanming Song on 6/30/25.
//

#include <AudioButtons.hpp>
#include <IAudioController.hpp>

using namespace TuneKey;

void PlayButton::interact(IAudioController* controller) const
{
    controller->play(getID(), getFilePath());
}

void PauseButton::interact(IAudioController* controller) const
{
    controller->pause(getID());
}

void ResumeButton::interact(IAudioController* controller) const
{
    controller->resume(getID());
}

void StopButton::interact(IAudioController* controller) const
{
    controller->stop(getID());
}
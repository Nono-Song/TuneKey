//
// Created by Schizoneurax on 3/11/2025.
//
#include "Button.h"

#include <utility>
#include "AudioController.h"
#include "ButtonManager.h"


Button::~Button() noexcept { this->release(); };

Button::Button(const uuid_t uuid, name_t name,
               ButtonManager* button_manager,
               AudioController* controller,
               const std::string& path)
    : name_(std::move(name)),
      uuid_(uuid),
      file_path_(path),
      audio_controller_(controller),
      button_manager_(button_manager)
{
};

void Button::execute(const ActionType& action, const std::any& params)
{
    switch (action)
    {
    case ActionType::Play:
        playAudio();
        break;
    case ActionType::Pause:
        pauseAudio();
        break;
    case ActionType::Resume:
        resumeAudio();
        break;
    case ActionType::Release:
        release();
        break;
    case ActionType::ModifyPath:
        modifyFilePath(std::any_cast<std::string>(params));
        break;
    case ActionType::ModifyName:
        modifyName(std::any_cast<name_t>(params));
        break;
    }
}

void Button::playAudio() const
{
    button_manager_->setActiveButton(uuid_);
    audio_controller_->play(file_path_);
}

void Button::pauseAudio() const
{
    if (button_manager_->getActiveButton() == uuid_)
    {
        audio_controller_->pause();
    }
}

void Button::resumeAudio() const
{
    if (button_manager_->getActiveButton() == uuid_)
    {
        audio_controller_->resume();
    }
}

void Button::release() const
{
    if (button_manager_->getActiveButton() == uuid_)
    {
        button_manager_->clearActiveButton();
        audio_controller_->stop();
    }
}

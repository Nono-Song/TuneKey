//
// Created by Schizoneurax on 3/11/2025.
//
#include "Button.h"
#include <stdexcept>


Button::~Button() noexcept { this->release(); };

Button::Button(uuid_t uuid, name_t name, std::string path)
    : name_(std::move(name)), uuid_(uuid), file_path_(std::move(path)) {
};

void Button::execute(const ActionType &action, const std::any &params) {
    switch (action) {
        case ActionType::Play:
            playAudio();
            break;
        case ActionType::Pause:
            pauseAudio();
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

void Button::playAudio() const {
    throw std::logic_error("Not implemented");
}

void Button::pauseAudio() const {
    throw std::logic_error("Not implemented");
}

void Button::release() {
    // TODO: Button::release()
}

void Button::modifyFilePath(std::string &&path) {
    this->release();
    file_path_ = path;
    throw std::logic_error("Not implemented");
}

//
// Created by Huanming Song on 6/25/25.
//

#ifndef SDLMANAGER_HPP
#define SDLMANAGER_HPP

#include <stdexcept>
#include <SDL3_mixer/SDL_mixer.h>
// An RAII SDL encapsulation
class SDLManager
{
public:
    SDLManager() = default;

    void init()
    {
        if (!SDL_Init(SDL_INIT_AUDIO))
        {
            throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
        }

        audio_device_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (!audio_device_)
        {
            throw std::runtime_error("SDL_OpenAudioDevice failed: " + std::string(SDL_GetError()));
        }

        if (!Mix_OpenAudio(audio_device_, nullptr))
        {
            throw std::runtime_error("Mix_OpenAudio failed: " + std::string(SDL_GetError()));
        }
    }

    ~SDLManager()
    {
        Mix_CloseAudio();
        SDL_CloseAudioDevice(audio_device_);
        Mix_Quit();
        SDL_Quit();
    }

    SDLManager(const SDLManager&) = delete;
    SDLManager& operator=(const SDLManager&) = delete;

    Mix_Music* load_and_play(const std::string& audio_path_)
    {
        music_ = Mix_LoadMUS(audio_path_.c_str());
        if (!music_ || !Mix_PlayMusic(music_, 0))
        {
            Mix_CloseAudio();
            throw std::runtime_error(!music_ ? "Mix_LoadMUS: " : "Mix_PlayMusic: " + std::string(SDL_GetError()));
        }

        return music_;
    }

private:
    SDL_AudioDeviceID audio_device_{};
    Mix_Music* music_{};
};
#endif //SDLMANAGER_HPP

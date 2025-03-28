//
// Created by Schizoneurax on 3/18/2025.
//

#include "AudioController.h"
#include <chrono>
#include <fmt/base.h>

using namespace std::literals::chrono_literals;

AudioController::AudioController()
{
}

AudioController::~AudioController()
{
    shutdown();
}

void AudioController::start()
{
    atomic_setter<&AudioController::curr_state_>(State::Idle);
    state_machine_thread_ = std::jthread{std::bind_front(&AudioController::stateMachineLoop, this)};
}

void AudioController::shutdown()
{
    atomic_setter<&AudioController::curr_state_>(State::Offline);
    state_machine_thread_.request_stop();
    state_change_epilogue();
}

void AudioController::play(boost::filesystem::path path)
{
    if (atomic_getter<&AudioController::curr_state_>() == State::Offline)
    {
        start();
    }
    else
    {
        stop();
    }

    {
        std::unique_lock lock(state_machine_mutex_);
        curr_audio_path_ = std::move(path);
        curr_state_ = State::Play;
    }
    state_change_epilogue();
}

void AudioController::pause()
{
    {
        std::unique_lock lock(state_machine_mutex_);
        if (curr_state_ == State::Play)
        {
            curr_state_ = State::Pause;
        }
    }
    state_change_epilogue();
}

void AudioController::resume()
{
    {
        std::unique_lock lock(state_machine_mutex_);
        if (curr_state_ == State::Pause)
        {
            curr_state_ = State::Play;
        }
    }
    state_change_epilogue();
}

void AudioController::stop()
{
    {
        std::unique_lock lock(state_machine_mutex_);
        if (curr_state_ == State::Offline)
        {
            return;
        }
        curr_state_ = State::Idle;
    }
    state_change_epilogue();
}


void AudioController::playAudio(const std::stop_token& stoken)
{
    // Play the audio and wait for possible pause or new play request
    const auto duration = duration_.load();
    for (auto i = 0; i < duration; ++i)
    {
        if (stoken.stop_requested())
        {
            std::puts("Audio Interrupted\n");
            audio_finished_naturally_.store(false);
            duration_.store(duration - i);
            return;
        }

        std::this_thread::sleep_for(1s);
        fmt::print("Audio Playing... {}s\n", (10 - duration) + i + 1);
    }

    duration_.store(0);

    std::puts("Audio finished\n");
    audio_finished_naturally_.store(true);
    state_machine_condition_.notify_one();
}

void AudioController::state_change_epilogue()
{
    state_machine_condition_.notify_one();
}


void AudioController::stateMachineLoop(const std::stop_token& stoken)
{
    while (!stoken.stop_requested())
    {
        // Will this mutex be destroyed and recreated in each iteration?
        std::unique_lock lock(state_machine_mutex_);
        switch (curr_state_)
        {
        case State::Offline:
            return;
        case State::Idle:
            state_machine_condition_.wait(lock, stoken, [this]() { return curr_state_ == State::Play; });
            break;
        case State::Play:
            // Play audio until it ends or interrupted by pause or new play request
            // Update metadata for the audio
            {
                if (audio_finished_naturally_.load())
                {
                    duration_.store(10);
                }

                audio_finished_naturally_.store(false);
                auto audio_thread = std::jthread([this]<typename T>(T&& PH1)
                {
                    playAudio(std::forward<T>(PH1));
                });

                state_machine_condition_.wait(lock, stoken, [this]()
                                              {
                                                  return curr_state_ != State::Play || audio_finished_naturally_.load();
                                              }
                );

                audio_thread.request_stop();
                if (audio_finished_naturally_.load())
                {
                    curr_state_ = State::Idle;
                }
            }
            break;
        case State::Pause:
            state_machine_condition_.wait(lock, stoken, [this]()
            {
                return curr_state_ != State::Pause;
            });
            break;
        }
    }
}

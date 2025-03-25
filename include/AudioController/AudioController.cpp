//
// Created by Schizoneurax on 3/18/2025.
//

#include "AudioController.h"
#include <chrono>
#include <fmt/core.h>
#include <fmt/chrono.h>

using namespace std::literals::chrono_literals;

AudioController::AudioController() : remaining_time_(10)
{
}

AudioController::~AudioController()
{
    shutdown();
}

void AudioController::start()
{
    state_machine_thread_ = std::jthread{std::bind_front(&AudioController::stateMachineLoop, this)};
}

void AudioController::shutdown()
{
    audio_thread_.request_stop();
    state_machine_thread_.request_stop();
    atomic_setter<&AudioController::curr_state_>(State::Idle);
    state_machine_condition_.notify_all();
}

void AudioController::play(boost::filesystem::path path)
{
    stop();
    {
        std::unique_lock lock(state_machine_mutex_);
        curr_audio_path_ = std::move(path);
        remaining_time_.store(10);
        curr_state_ = State::Play;
    }
    state_machine_condition_.notify_one();
    std::this_thread::sleep_for(20ms);
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
    state_machine_condition_.notify_one();
    std::this_thread::sleep_for(20ms);
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
    state_machine_condition_.notify_one();
    std::this_thread::sleep_for(20ms);
}

void AudioController::stop()
{
    atomic_setter<&AudioController::curr_state_>(State::Idle);
    state_machine_condition_.notify_one();
    std::this_thread::sleep_for(20ms);
}


void AudioController::playAudio(const std::stop_token& stoken)
{
    if (atomic_getter<&AudioController::curr_state_>() != State::Play)
    {
        return;
    }

    // Todo: Need to Update metadata for the audio
    auto filepath = atomic_getter<&AudioController::curr_audio_path_>();

    // Play the audio and wait for possible pause or new play request
    while (!stoken.stop_requested() &&
        remaining_time_.load() > 0 &&
        atomic_getter<&AudioController::curr_state_>() == State::Play)
    {
        std::this_thread::sleep_for(1s);
        remaining_time_.fetch_sub(1);
        fmt::print("Remaining time: {}s\n", remaining_time_.load());
    }

    if (remaining_time_.load() > 0)
    {
        std::puts("Audio interrupted\n");
    }
    else
    {
        std::puts("Audio finished\n");
        atomic_setter<&AudioController::curr_state_>(State::Idle);
        state_machine_condition_.notify_one();
    }
}


void AudioController::stateMachineLoop(const std::stop_token& stoken)
{
    while (!stoken.stop_requested())
    {
        // Will this mutex be destroyed and recreated in each iteration?
        std::shared_lock lock(state_machine_mutex_);
        switch (curr_state_)
        {
        case State::Idle:
            state_machine_condition_.wait(lock, stoken, [this]() { return curr_state_ == State::Play; });
            break;
        case State::Play:
            // Play audio until it ends or interrupted by pause or new play request
            // Update metadata for the audio

            audio_thread_ = std::jthread(std::bind_front(&AudioController::playAudio, this));
            state_machine_condition_.wait(lock, stoken, [this]()
                                          {
                                              return curr_state_ != State::Play;
                                          }
            );

            if (audio_thread_.get_stop_token().stop_possible())
            {
                audio_thread_.request_stop();
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

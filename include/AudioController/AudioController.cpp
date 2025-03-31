//
// Created by Schizoneurax on 3/18/2025.
//

#include "AudioController.h"
#include <chrono>
#include <fmt/base.h>

using namespace std::literals::chrono_literals;

AudioController::AudioController() = default;
AudioController::~AudioController() = default;

void AudioController::start()
{
    curr_state_ = State::Idle; // Assume only a single thread is running right not
    state_machine_thread_ = std::jthread{std::bind_front(&AudioController::stateMachineLoop, this)};
}

void AudioController::shutdown() const
{
    push_command<ShutdownCmd>();
}

void AudioController::play(const boost::filesystem::path& path) const
{
    push_command<PlayCmd>(path);
}

void AudioController::pause() const
{
    push_command<PauseCmd>();
}

void AudioController::resume() const
{
    push_command<ResumeCmd>();
}

void AudioController::stop() const
{
    push_command<StopCmd>();
}


void AudioController::playAudio(const std::stop_token& stoken)
{
    std::unique_lock lock(audio_mutex_);
    // Play the audio and wait for possible pause or new play request
    const auto duration = duration_.load();
    for (auto i = 0; i < duration; ++i)
    {
        if (audio_paused_.load())
        {
            std::puts("Audio Paused\n");
            audio_condition_.wait(lock, stoken, [this]() { return !audio_paused_.load(); });
        }

        if (stoken.stop_requested())
        {
            std::puts("Audio Interrupted\n");
            return;
        }

        std::this_thread::sleep_for(1s);
        fmt::print("Audio Playing... {}s\n", i + 1);
    }

    std::puts("Audio finished\n");
}


void AudioController::stateMachineLoop(const std::stop_token& stoken)
{
    std::stop_callback cb(stoken, [this]()
    {
        if (audio_thread_.get_stop_source().stop_possible())
        {
            audio_thread_.request_stop();
            audio_thread_.join();
        }
    });

    event_queue_ = std::make_unique<EventQueue<Command>>(stoken);

    while (!stoken.stop_requested())
    {
        try
        {
            std::visit([this]<typename T>(const T& arg)
            {
                std::unique_lock lock(state_machine_mutex_);
                using U = std::decay_t<T>;
                if constexpr (std::is_same_v<T, PlayCmd>)
                {
                    if (curr_state_ != State::Idle)
                    {
                        audio_thread_.request_stop();
                        audio_thread_.join();
                    }

                    curr_state_ = State::Play;
                    curr_audio_path_ = arg.path;
                    audio_thread_ = std::jthread{std::bind_front(&AudioController::playAudio, this)};
                }
                else if constexpr (std::is_same_v<U, ShutdownCmd>)
                {
                    if (curr_state_ != State::Offline)
                    {
                        curr_state_ = State::Offline;
                        state_machine_thread_.request_stop();
                    }
                }
                else if constexpr (std::is_same_v<U, PauseCmd>)
                {
                    if (curr_state_ == State::Play)
                    {
                        curr_state_ = State::Pause;
                        audio_paused_.store(true);
                    }
                }
                else if constexpr (std::is_same_v<U, ResumeCmd>)
                {
                    if (curr_state_ == State::Pause)
                    {
                        curr_state_ = State::Play;
                        audio_paused_.store(false);
                        audio_condition_.notify_one();
                    }
                }
                // if constexpr (std::is_same_v<U, StopCmd>)
                else
                {
                    if (curr_state_ != State::Offline && curr_state_ != State::Idle)
                    {
                        curr_state_ = State::Idle;
                        audio_thread_.request_stop();
                        audio_thread_.join();
                    }
                }
            }, pop_command());
        }
        catch (...)
        {
            break;
        }
    }

    event_queue_.reset();
}

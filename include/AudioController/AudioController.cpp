//
// Created by Schizoneurax on 3/18/2025.
//

#include "AudioController.h"
#include <chrono>
#include <fmt/base.h>

using namespace std::literals::chrono_literals;

// @formatter:off
template <typename... Args>
struct Visitor: Args... { using Args::operator()...; };

// @formatter:on
AudioController::AudioController() = default;

AudioController::~AudioController()
{
    if (std::shared_lock l(state_machine_mutex_); curr_state_ != State::Offline)
    {
        l.unlock();
        shutdown();
    }
}

void AudioController::start()
{
    if (std::shared_lock l(state_machine_mutex_);
        curr_state_ != State::Offline
    )
    {
        throw std::runtime_error("AudioController::start() failed: Instance running");
    }

    reset_playback();

    audio_thread_ = std::jthread(
        [this](const std::stop_token& stoken)
        {
            try { playAudio(stoken); }
            catch (std::exception& e)
            {
                fmt::print("playAudioLoop: {}\n", e.what());
                // Todo: exception handling
            }
        });

    state_machine_thread_ = std::jthread{
        [this](const std::stop_token& stoken)
        {
            event_queue_ = std::make_unique<EventQueue<Command>>(stoken);
            stateMachineLoop(stoken);
        }
    };
}

void AudioController::shutdown()
{
    if (state_machine_thread_.joinable())
    {
        if (std::shared_lock lock(state_machine_mutex_);
            curr_state_ != State::Offline)
        {
            push_command<ShutdownCmd>();
        }
        state_machine_thread_.join();
    }
}

void AudioController::play(const boost::filesystem::path& path)
{
    push_command<PlayCmd>(path);
}

void AudioController::pause()
{
    push_command<PauseCmd>();
}

void AudioController::resume()
{
    push_command<ResumeCmd>();
}

void AudioController::stop()
{
    push_command<StopCmd>();
}

void AudioController::playAudio(const std::stop_token& stoken)
{
    uint64_t playback_id = 0;
    while (!stoken.stop_requested())
    {
        std::shared_lock lock(state_machine_mutex_);
        audio_condition_.wait(lock, stoken, [this]()
        {
            return curr_state_ == State::Play;
        });
        if (stoken.stop_requested()) { break; }

        const auto audio_path = curr_audio_path_;
        playback_id = curr_playback_id_.load(std::memory_order_acquire);

        constexpr unsigned interval = 50;
        constexpr auto ratio = 1000 / interval;
        const auto duration = duration_.load(std::memory_order_acquire) * ratio;

        lock.unlock();


        // Simulate audio playback
        for (unsigned i = 0; i < duration; i++)
        {
            // Play a new audio if playback id changes
            if (playback_id != curr_playback_id_.load(std::memory_order_acquire))
            {
                fmt::print("AudioThread: playback id {} superseded by {}\n",
                           playback_id, curr_playback_id_.load(std::memory_order_acquire));
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if (i % ratio == 0)
            {
                fmt::print("Audio playing... {}s\n", i / ratio + 1);
            }

            if (std::shared_lock pause_lock(state_machine_mutex_);
                curr_state_ == State::Pause)
            {
                fmt::print("Audio paused...\n");

                audio_condition_.wait(pause_lock, stoken, [this]()
                {
                    return curr_state_ != State::Pause;
                });

                if (curr_state_ == State::Play)
                {
                    fmt::print("Audio resumed...\n");
                }
            }

            if (std::shared_lock stop_lock(state_machine_mutex_);
                curr_state_ == State::Idle || stoken.stop_requested())
            {
                break;
            }
        }

        if (std::shared_lock state_lock(state_machine_mutex_);
            curr_state_ == State::Idle || stoken.stop_requested())
        {
            fmt::print("Audio interrupted...\n");
        }
        else if (playback_id == curr_playback_id_.load(std::memory_order_acquire))
        {
            fmt::print("Audio finished naturally\n");
            push_command<StopCmd>();
        }
    }
}

void AudioController::reset_playback()
{
    // Reset all playback metadata for a new playback
    curr_state_ = State::Idle;
    curr_audio_path_ = "";
    duration_ = default_duration;
}

void AudioController::play_callback(const PlayCmd& play_cmd)
{
    {
        std::scoped_lock lock(state_machine_mutex_);
        reset_playback();
        fmt::print("Playing new audio...\n");
        ++curr_playback_id_;
        curr_state_ = State::Play;
        curr_audio_path_ = play_cmd.path;
    }

    audio_condition_.notify_one();
}

void AudioController::pause_callback(const PauseCmd&)
{
    if (std::unique_lock lock(state_machine_mutex_);
        curr_state_ == State::Play)
    {
        curr_state_ = State::Pause;
    }
}

void AudioController::resume_callback(const ResumeCmd&)
{
    if (std::unique_lock lock(state_machine_mutex_);
        curr_state_ == State::Pause)
    {
        curr_state_ = State::Play;
        audio_condition_.notify_one();
    }
}

void AudioController::stop_callback(const StopCmd&)
{
    if (std::unique_lock lock(state_machine_mutex_);
        curr_state_ != State::Idle && curr_state_ != State::Offline)
    {
        reset_playback();
        audio_condition_.notify_one();
    }
}

void AudioController::shutdown_callback(const ShutdownCmd&)
{
    if (std::unique_lock lock(state_machine_mutex_);
        curr_state_ != State::Offline)
    {
        curr_state_ = State::Offline;
        curr_playback_id_ = 0;
        lock.unlock();

        if (audio_thread_.joinable())
        {
            audio_thread_.request_stop();
            audio_condition_.notify_one();
            audio_thread_.join();
        }
        state_machine_thread_.request_stop();
    }
}


void AudioController::stateMachineLoop(const std::stop_token& stoken) noexcept
{
    static const auto visit = Visitor{
        [this](const PlayCmd& play_cmd) { play_callback(play_cmd); },
        [this](const PauseCmd& pause_cmd) { pause_callback(pause_cmd); },
        [this](const ResumeCmd& resume_cmd) { resume_callback(resume_cmd); },
        [this](const StopCmd& stop_cmd) { stop_callback(stop_cmd); },
        [this](const ShutdownCmd& shutdown_cmd) { shutdown_callback(shutdown_cmd); },
    };

    while (!stoken.stop_requested())
    {
        try
        {
            auto evt = pop_command();
            std::visit(visit, evt);
        }
        catch (std::exception& e)
        {
            fmt::print("stateMachineLoop: Exception occurred: {}\n", e.what());
            break;
        }
    }
}

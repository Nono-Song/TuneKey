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
                shutdown();
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
    while (!stoken.stop_requested())
    {
        std::shared_lock lock(state_machine_mutex_);

        audio_waiting_.store(true, std::memory_order_release);
        audio_condition_.notify_one();
        audio_condition_.wait(lock, stoken, [this]()
        {
            return curr_state_ == State::Play;
        });
        audio_waiting_.store(false, std::memory_order_release);

        if (stoken.stop_requested()) { break; }

        const auto audio_path = curr_audio_path_;

        constexpr unsigned interval = 50;
        constexpr auto ratio = 1000 / interval;
        const auto duration = duration_.load(std::memory_order_acquire) * ratio;

        lock.unlock();


        // Simulate audio playback
        for (unsigned i = 0; i <= duration; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if (i % ratio == 0)
            {
                fmt::print("Audio playing... {}s\n", i / ratio + 1);
            }

            if (std::shared_lock l(state_machine_mutex_);
                curr_state_ == State::Pause)
            {
                fmt::print("Audio paused...\n");

                audio_condition_.wait(l, stoken, [this]()
                {
                    return curr_state_ != State::Pause;
                });

                if (curr_state_ == State::Play)
                {
                    fmt::print("Audio resumed...\n");
                }
            }

            if (std::shared_lock l(state_machine_mutex_);
                curr_state_ == State::Idle || stoken.stop_requested())
            {
                break;
            }
        }

        if (std::scoped_lock l(state_machine_mutex_);
            curr_state_ == State::Idle || stoken.stop_requested())
        {
            fmt::print("Audio interrupted...\n");
        }
        else
        {
            fmt::print("Audio finished naturally\n");
            curr_state_ = State::Idle;
        }
    }
}

void AudioController::reset_playback() noexcept
{
    // Reset all playback metadata for a new playback
    curr_state_ = State::Idle;
    curr_audio_path_ = "";
    audio_waiting_.store(false, std::memory_order_release);
    duration_.store(default_duration, std::memory_order_release);
}

void AudioController::play_callback(const PlayCmd& play_cmd)
{
    {
        std::unique_lock lock(state_machine_mutex_);
        if (curr_state_ != State::Idle)
        {
            reset_playback();
            audio_condition_.notify_one();
            audio_condition_.wait(lock, [this]()
            {
                return audio_waiting_.load(std::memory_order_acquire);
            });
            fmt::print("Playing new audio...\n");
        }

        // Set new playback information
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

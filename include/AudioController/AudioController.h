//
// Created by Schizoneurax on 3/18/2025.
//

#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <boost/filesystem.hpp>
#include <utility>
#include <variant>
#include "EventQueue.hpp"

class AudioController
{
public:
    AudioController();
    ~AudioController();
    AudioController(const AudioController&) = delete;
    AudioController& operator=(const AudioController&) = delete;
    AudioController(AudioController&&) = delete;
    AudioController& operator=(AudioController&&) = delete;

    void start();
    void shutdown() const;

    // Change state
    void play(const boost::filesystem::path& path) const;
    void pause() const;
    void resume() const;
    void stop() const;

    void stateMachineLoop(const std::stop_token& stoken);

private:
    /** Command **/
    struct PlayCmd
    {
        PlayCmd() = default;

        explicit PlayCmd(boost::filesystem::path path): path(std::move(path))
        {
        }

        PlayCmd(const PlayCmd& other) = default;

        PlayCmd(PlayCmd&& other) noexcept: path(std::move(other.path))
        {
        }

        boost::filesystem::path path;
    };

    struct PauseCmd
    {
    };

    struct ResumeCmd
    {
    };

    struct StopCmd
    {
    };

    struct ShutdownCmd
    {
    };

    using Command = std::variant<PlayCmd,
                                 PauseCmd,
                                 ResumeCmd,
                                 StopCmd,
                                 ShutdownCmd>;

    template <typename Cmd, typename... Args>
    void push_command(Args&&... args) const
    {
        event_queue_->push(Cmd(std::forward<Args>(args)...));
    }

    auto pop_command() const
    {
        return event_queue_->pop();
    }

    /** State **/
    enum class State
    {
        Offline, Idle, Play, Pause
    };

    // To SDL2?
    void playAudio(const std::stop_token& stoken);
    // Todo: void pauseAudio() const;
    // Todo: void resumeAudio() const;

    /** Synchronization **/
    std::jthread state_machine_thread_;
    std::jthread audio_thread_;
    mutable std::mutex audio_mutex_;
    mutable std::shared_mutex state_machine_mutex_;
    std::condition_variable_any audio_condition_;

    /** Shared Data **/
    State curr_state_{State::Offline};
    std::atomic_int duration_{10};
    std::atomic_bool audio_paused_{false};
    boost::filesystem::path curr_audio_path_{};
    std::unique_ptr<EventQueue<Command>> event_queue_{};
};


#endif //AUDIOCONTROLLER_H

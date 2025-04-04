//
// Created by Schizoneurax on 3/18/2025.
//

#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H
#include <thread>
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
    void shutdown();

    // Change state
    void play(const boost::filesystem::path& path);
    void pause();
    void resume();
    void stop();

    void stateMachineLoop(const std::stop_token& stoken) noexcept;

private:
    // @formatter:off
    /** Command **/
    struct PlayCmd
    {
        explicit PlayCmd(boost::filesystem::path path): path(std::move(path)) {}
        boost::filesystem::path path;
    };
    struct PauseCmd{};
    struct ResumeCmd{};
    struct StopCmd{};
    struct ShutdownCmd{};

    using Command = std::variant<PlayCmd,
                                 PauseCmd,
                                 ResumeCmd,
                                 StopCmd,
                                 ShutdownCmd>;

    // @formatter:on
    template <typename Cmd, typename... Args>
    void push_command(Args&&... args)
    {
        if (event_queue_)
        {
            return event_queue_->push(Cmd(std::forward<Args>(args)...));
        }
        throw QueueStoppedException{};
    }

    Command pop_command()
    {
        try
        {
            if (event_queue_)
            {
                return event_queue_->pop();
            }
        }
        catch (const QueueStoppedException&)
        {
            if (curr_state_ != State::Offline)
            {
                throw std::runtime_error{"EventQueue stopped unexpectedly"};
            }
        }

        return ShutdownCmd{};
    }

    /** State **/
    enum class State { Offline, Idle, Play, Pause };

    void playAudio(const std::stop_token& stoken);

    void reset_playback() noexcept;
    void play_callback(const PlayCmd& play_cmd);
    void pause_callback(const PauseCmd& pause_cmd);
    void resume_callback(const ResumeCmd& resume_cmd);
    void stop_callback(const StopCmd& stop_cmd);
    void shutdown_callback(const ShutdownCmd& shutdown_cmd);

    static constexpr int default_duration{10};

    /** Synchronization **/
    std::jthread state_machine_thread_;
    std::jthread audio_thread_;
    mutable std::shared_mutex state_machine_mutex_;
    std::condition_variable_any audio_condition_;

    /** Shared Data **/
    std::atomic_int duration_{default_duration};
    std::atomic_bool audio_waiting_{false};
    State curr_state_{State::Offline};
    boost::filesystem::path curr_audio_path_{};
    std::unique_ptr<EventQueue<Command>> event_queue_{};
};


#endif //AUDIOCONTROLLER_H

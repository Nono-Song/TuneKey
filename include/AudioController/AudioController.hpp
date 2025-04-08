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

private:
    // @formatter:off
    /** Command **/
    struct PlayEvent
    {
        explicit PlayEvent(boost::filesystem::path path): path(std::move(path)) {}
        boost::filesystem::path path;
    };
    struct PauseEvent{};
    struct ResumeEvent{};
    struct StopEvent{};
    struct ShutdownEvent{};
    struct AudioFinishedEvent{};
    struct AudioErrorEvent{};

    // @formatter:on
    using Command = std::variant<PlayEvent,
                                 PauseEvent, ResumeEvent,
                                 StopEvent, ShutdownEvent,
                                 AudioFinishedEvent,
                                 AudioErrorEvent>;

    template <typename Cmd, typename... Args>
    void push_event(Args&&... args);

    Command pop_event();

    /** State **/
    enum class State { Offline, Idle, Play, Pause };

    // In case of audio error and need to just restart the audio thread...
    void start_audio_thread() noexcept;
    // Audio thread main loop
    void audio_event_loop(const std::stop_token&);
    void state_machine_loop(const std::stop_token& stoken) noexcept;

    void reset_playback();

    // State change callback functions
    void play_callback(const PlayEvent&);
    void pause_callback(const PauseEvent&);
    void resume_callback(const ResumeEvent&);
    void stop_callback(const StopEvent&);
    void audio_finished_callback(const AudioFinishedEvent&);
    void shutdown_callback(const ShutdownEvent&);
    void error_callback(const AudioErrorEvent&);

    static constexpr int default_duration{10};

    std::stop_source machine_ssource_{};
    std::unique_ptr<EventQueue<Command>> event_queue_{};
    mutable std::shared_mutex state_machine_mutex_{};
    State curr_state_{State::Offline};
    boost::filesystem::path curr_audio_path_{};
    std::atomic_uint64_t curr_playback_id_{0};
    std::atomic_int duration_{default_duration};
    std::condition_variable_any audio_condition_{};

    std::jthread state_machine_thread_;
    std::jthread audio_thread_{};
};

template <typename Cmd, typename... Args>
void AudioController::push_event(Args&&... args)
{
    if (event_queue_)
    {
        return event_queue_->push(Cmd{std::forward<Args>(args)...});
    }
    throw QueueStoppedException{};
}


#endif //AUDIOCONTROLLER_H

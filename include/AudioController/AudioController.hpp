//
// Created by Schizoneurax on 3/18/2025.
//

#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H
#include <thread>
#include <shared_mutex>
#include <condition_variable>
#include <optional>
#include <boost/filesystem.hpp>
#include <utility>
#include <Event.hpp>
#include <EventQueue.hpp>

class AudioController
{
public:
    AudioController(std::unique_ptr<EventQueue<Event>>&);
    ~AudioController();
    AudioController(const AudioController&) = delete;
    AudioController& operator=(const AudioController&) = delete;
    AudioController(AudioController&&) = delete;
    AudioController& operator=(AudioController&&) = delete;

    void start();
    void shutdown();

    // Change state
    void play(const identifier_type id, const boost::filesystem::path& path);
    void pause(const identifier_type id);
    void resume(const identifier_type id);
    void stop(const identifier_type id);

private:
    template <typename Cmd, typename... Args>
    void push_event(Args&&... args);
    Event pop_event();

    /** State **/
    enum class State { Error, Offline, Idle, Play, Pause };

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
    std::unique_ptr<EventQueue<Event>>& event_queue_;
    mutable std::shared_mutex state_machine_mutex_{};
    mutable std::shared_mutex callback_mutex_{};
    State curr_state_{State::Offline};
    boost::filesystem::path curr_audio_path_{};
    std::optional<identifier_type> curr_playback_id_{};
    std::atomic_int duration_{default_duration};
    std::condition_variable_any audio_condition_{};

    std::jthread state_machine_thread_{};
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

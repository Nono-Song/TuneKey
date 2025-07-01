//
// Created by Schizoneurax on 4/10/2025.
//

#pragma once

#include <thread>
#include <shared_mutex>
#include <condition_variable>
#include <future>
#include <optional>
#include <variant>
#include "IAudioController.hpp"

namespace TuneKey
{
    template <typename T>
    class EventQueue;

    struct PlayEvent;
    struct PauseEvent;
    struct StopEvent;
    struct ResumeEvent;
    struct AudioReadyEvent;
    struct AudioFinishedEvent;
    struct AudioErrorEvent;
    struct ShutdownEvent;

    /** event_t related types **/
    using event_t = std::variant<PlayEvent,
        PauseEvent,
        ResumeEvent,
        StopEvent,
        AudioReadyEvent,
        AudioFinishedEvent,
        AudioErrorEvent,
        ShutdownEvent>;

    class AudioControllerImpl: public IAudioController
    {
    public:
        AudioControllerImpl();
        ~AudioControllerImpl() override;
        AudioControllerImpl(const AudioControllerImpl&) = delete;
        AudioControllerImpl& operator=(const AudioControllerImpl&) = delete;
        AudioControllerImpl(AudioControllerImpl&&) = delete;
        AudioControllerImpl& operator=(AudioControllerImpl&&) = delete;

        void start() override;
        void shutdown() override;

        // Change state
        void play(identifier_type id, const filename_type& path) override;
        void pause(identifier_type id) override;
        void resume(identifier_type id) override;
        void stop(identifier_type id) override;

        std::optional<identifier_type> active_button() const override;

    private:
        event_t pop_event();

        /** State **/
        enum class State {Init, Error, Offline, Idle, Play, Pause };

        // In case of audio error and need to just restart the audio thread...
        void start_audio_thread() noexcept;
        // Audio thread main loop
        void audio_event_loop(const std::stop_token&);
        void state_machine_loop(const std::stop_token&) noexcept;

        // State change callback functions
        void play_callback(const PlayEvent&);
        void pause_callback(const PauseEvent&);
        void resume_callback(const ResumeEvent&);
        void stop_callback(const StopEvent&);
        void audio_ready_callback(const AudioReadyEvent&);
        void audio_finished_callback(const AudioFinishedEvent&);
        void shutdown_callback(const ShutdownEvent&);
        void error_callback(const AudioErrorEvent&);

        static constexpr int default_duration{10};

        std::unique_ptr<EventQueue<event_t>> event_queue_;
        mutable std::shared_mutex state_machine_mutex_{};
        State curr_state_{State::Offline};
        filename_type curr_audio_path_{};
        std::optional<identifier_type> curr_active_button_{};

        /**
         * The playback_id makes clear if we need to play a new audio
         **/
        std::optional<identifier_type> curr_playback_id_{};
        std::condition_variable_any audio_condition_{};

        std::jthread state_machine_thread_{};
        std::jthread audio_thread_{};

        std::promise<void> audio_ready_;
    };
}

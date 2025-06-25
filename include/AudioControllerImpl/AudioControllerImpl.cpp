//
// Created by Schizoneurax on 4/10/2025.
//

#include "AudioControllerImpl.hpp"
#include <cassert>
#include <chrono>
#include <fmt/base.h>
#include <SDLManager.hpp>
#include "EventQueue.hpp"

using namespace std::literals::chrono_literals;

inline std::optional<identifier_type>& operator++(std::optional<identifier_type>& id)
{
    id = id.transform([](identifier_type op)
    {
        return ++op;
    });
    return id;
}

// @formatter:off
template <typename... Args>
struct Visitor: Args... { using Args::operator()...; };

AudioControllerImpl::AudioControllerImpl()
: event_queue_{std::make_unique<EventQueue<Event>>()} {}

// @formatter:on
AudioControllerImpl::~AudioControllerImpl()
{
    if (std::shared_lock l(state_machine_mutex_);
        curr_state_ != State::Offline)
    {
        l.unlock();
        AudioControllerImpl::shutdown();
    }
}

Event AudioControllerImpl::pop_event()
{
    if (event_queue_)
    {
        return event_queue_->pop();
    }

    throw std::runtime_error("Event queue not initialized!");
}

void AudioControllerImpl::start_audio_thread() noexcept
{
    curr_playback_id_ = 0;
    curr_active_button_.reset();
    audio_thread_ = std::jthread([this](const std::stop_token& stoken)
    {
        try { audio_event_loop(stoken); }
        catch (std::exception& e)
        {
            event_queue_->push(AudioErrorEvent{
                std::nullopt,
                std::string(e.what()) + "\n"
            });
        }
    });
}

void AudioControllerImpl::start()
{
    std::unique_lock l(state_machine_mutex_, std::try_to_lock);
    if (!l.owns_lock() || curr_state_ != State::Offline ||
        audio_thread_.joinable() ||
        state_machine_thread_.joinable())
    {
        throw std::runtime_error("AudioController::start() failed: Instance running");
    }

    if (!event_queue_)
    {
        throw std::runtime_error("AudioControllerImpl::start() failed: No event_queue");
    }

    curr_state_ = State::Init;
    l.unlock();

    audio_ready_ = std::promise<void>{};
    auto fut = audio_ready_.get_future();

    start_audio_thread();
    state_machine_thread_ = std::jthread{
        [this](const std::stop_token& stoken)
        {
            // state_machine_loop doesn't throw exception
            state_machine_loop(stoken);
        }
    };

    fut.get();
}

void AudioControllerImpl::shutdown()
{
    if (std::shared_lock lock(state_machine_mutex_);
        curr_state_ != State::Offline)
    {
        event_queue_->push(ShutdownEvent{});
        lock.unlock();
        // While we can't call join() while handling the ShutdownEvent,
        // we can join() here.
        if (state_machine_thread_.joinable())
        {
            state_machine_thread_.join();
        }
    }

    fmt::print(stderr, "AudioControllerImpl::shutdown()\n");
}

void AudioControllerImpl::play(const identifier_type id, const filename_type& path)
{
    if (std::unique_lock l(state_machine_mutex_);
        curr_state_ == State::Play ||
        curr_state_ == State::Pause ||
        curr_state_ == State::Idle)
    {
        curr_active_button_ = id;
        l.unlock();
        event_queue_->push(PlayEvent{id, path});
    }
}

void AudioControllerImpl::pause(const identifier_type id)
{
    if (std::shared_lock l(state_machine_mutex_);
        curr_state_ == State::Play)
    {
        event_queue_->push(PauseEvent{id});
    }
}

void AudioControllerImpl::resume(const identifier_type id)
{
    if (std::shared_lock l(state_machine_mutex_);
        curr_state_ == State::Pause)
    {
        event_queue_->push(ResumeEvent{id});
    }
}

void AudioControllerImpl::stop(const identifier_type id)
{
    if (std::shared_lock l(state_machine_mutex_);
        curr_state_ == State::Play || curr_state_ == State::Pause)
    {
        event_queue_->push(StopEvent{id});
    }
}

std::optional<identifier_type> AudioControllerImpl::active_button() const
{
    std::shared_lock l{state_machine_mutex_};
    return curr_active_button_;
}

void AudioControllerImpl::audio_event_loop(const std::stop_token& stoken)
{
    SDLManager sdl_manager{};
    sdl_manager.init();
    event_queue_->push(AudioReadyEvent{});

    identifier_type playback_id = 0;
    while (!stoken.stop_requested())
    {
        std::shared_lock l{state_machine_mutex_};
        // Wait for any notification for 200ms
        audio_condition_.wait_for(l, 200ms);

        if (stoken.stop_requested()) { break; }

        // 1. Play an audio or switch to a new audio
        if (curr_state_ == State::Play && playback_id != curr_playback_id_.value_or(playback_id))
        {
            Mix_HaltMusic();
            try
            {
                sdl_manager.load_and_play(curr_audio_path_);
                playback_id = curr_playback_id_.value_or(playback_id);
            }
            catch (std::exception& e)
            {
                l.unlock();
                // Failed to load and play the music
                // It's not a critical error so we just pretend we've finished the playback
                // TODO: A more fine-grained error control
                fmt::print(stderr, "AudioControllerImpl::audio_event_loop(): {}\n", e.what());
                event_queue_->push(AudioFinishedEvent{playback_id});
            }
        }
        // 2. Resume the audio
        else if (curr_state_ == State::Play && Mix_PausedMusic())
        {
            Mix_ResumeMusic();
        }
        // 3. Pause the audio
        else if (curr_state_ == State::Pause && (Mix_PlayingMusic() && !Mix_PausedMusic()))
        {
            Mix_PauseMusic();
        }
        // 4. Stop the audio
        else if (curr_state_ == State::Idle && Mix_PlayingMusic())
        {
            Mix_HaltMusic();
        }
        // 5. Audio Finished
        else if (curr_state_ == State::Play && !Mix_PlayingMusic())
        {
            l.unlock();
            event_queue_->push(AudioFinishedEvent{playback_id});
            l.lock();
            audio_condition_.wait(l, stoken, [this] { return curr_state_ != State::Play; });
        }
    }
}

void AudioControllerImpl::play_callback(const PlayEvent& play_evt)
{
    // Play a new audio, irrespective of the current state,
    // except the machine has already shutdown, aka. in offline state.
    if (std::scoped_lock l(state_machine_mutex_);
        curr_state_ == State::Play ||
        curr_state_ == State::Pause ||
        curr_state_ == State::Idle)
    {
        fmt::print("Playing new audio...\n");
        // This is critical for the audio thread to distinguish a new audio from its current audio,
        ++curr_playback_id_;
        curr_state_ = State::Play;

        curr_audio_path_ = play_evt.filename;
        // Right now we need this only if the audio is paused. However, in actual
        // implementation the audio thread may also be in sleep while playing.
        audio_condition_.notify_one();
    }
}

void AudioControllerImpl::pause_callback(const PauseEvent&)
{
    // Pause the playback while keep all metadata.
    // Only makes sense if we are actually playing the audio
    if (std::unique_lock lock(state_machine_mutex_);
        curr_state_ == State::Play)
    {
        curr_state_ = State::Pause;
    }
}

void AudioControllerImpl::resume_callback(const ResumeEvent&)
{
    // Resume playback. Only makes sense if the playback is actually paused.
    if (std::scoped_lock lock(state_machine_mutex_);
        curr_state_ == State::Pause)
    {
        curr_state_ = State::Play;
        audio_condition_.notify_one();
    }
}

void AudioControllerImpl::stop_callback(const StopEvent&)
{
    // Manually stop the audio. The audio could be playing or paused.
    // After StopEvent, only a PlayEvent or ShutdownEvent will trigger a state change.
    if (std::unique_lock lock(state_machine_mutex_);
        curr_state_ == State::Pause || curr_state_ == State::Play)
    {
        curr_state_ = State::Idle;
        // curr_active_button_.reset();

        // See comment in play_callback
        audio_condition_.notify_one();
    }
}

void AudioControllerImpl::audio_ready_callback(const AudioReadyEvent&)
{
    if (std::scoped_lock lock(state_machine_mutex_);
        curr_state_ == State::Init || curr_state_ == State::Error)
    {
        curr_state_ = State::Idle;
        audio_condition_.notify_one();
    }
}


void AudioControllerImpl::audio_finished_callback(const AudioFinishedEvent&)
{
    // Just as the name suggests, AudioFinishEvent should only arrive when the audio
    // has finished playing. Currently, there is no way to know if the audio's truly
    // finished as we only simulate the audio playback here. However, according to the
    // logic of the code, this command is only fired inside the correct position of audio_event_loop()
    if (std::scoped_lock lock(state_machine_mutex_);
        curr_state_ == State::Play)
    {
        curr_state_ = State::Idle;
        fmt::print(stderr, "AudioControllerImpl::audio_finished_callback()\n");
        audio_condition_.notify_one();
    }
}

void AudioControllerImpl::shutdown_callback(const ShutdownEvent&)
{
    // Completely shutdown the state machine loop and the audio thread.
    // Need to call start() again to restart.
    if (std::unique_lock lock(state_machine_mutex_);
        curr_state_ != State::Offline)
    {
        curr_state_ = State::Offline;
        // Need to release the lock to avoid deadlock
        lock.unlock();

        if (audio_thread_.joinable())
        {
            audio_thread_.request_stop();
            audio_condition_.notify_one();
            audio_thread_.join();
        }

        // Shouldn't join the current thread, just request stop.
        state_machine_thread_.request_stop();
    }
}

void AudioControllerImpl::error_callback(const AudioErrorEvent&)
{
    // Add things when we actually implement the audio playback functionality
    // The argument is not used right now but in future it may contain information
    {
        std::scoped_lock lock(state_machine_mutex_);
        curr_state_ = State::Error;
    }
    // The audio_thread now won't block indefinitely so a deadlock won't happen
    if (audio_thread_.joinable())
    {
        audio_thread_.request_stop();
        audio_thread_.join();
    }

    start_audio_thread();
}


void AudioControllerImpl::state_machine_loop(const std::stop_token& stoken) noexcept
{
    static const auto visitor = Visitor{
        [this](const PlayEvent& evt) { play_callback(evt); },
        [this](const PauseEvent& evt) { pause_callback(evt); },
        [this](const ResumeEvent& evt) { resume_callback(evt); },
        [this](const StopEvent& evt) { stop_callback(evt); },
        [this](const AudioReadyEvent& evt) { audio_ready_callback(evt); },
        [this](const AudioFinishedEvent& evt) { audio_finished_callback(evt); },
        [this](const ShutdownEvent& evt) { shutdown_callback(evt); },
        [this](const AudioErrorEvent& evt) { error_callback(evt); },
    };



    const auto loop_until = [this, stoken](const State state)
    {
        while (!stoken.stop_requested())
        {
            try
            {
                std::visit(visitor, pop_event());
                if (std::shared_lock lock(state_machine_mutex_);
                    curr_state_ == state)
                    { break; }
            }
            catch (std::exception& e)
            {
                fmt::print("state_machine_loop: Exception occurred: {}\n", e.what());
            }
        }
    };

    /*
    if (std::shared_lock lock(state_machine_mutex_);
        curr_state_ == State::Offline)
    {
        audio_ready_.set_exception(std::make_exception_ptr(std::runtime_error("State is not Idle")));
        return;
    }
    */

    loop_until(State::Idle);

    assert(curr_state_ == State::Idle);
    audio_ready_.set_value();

    loop_until(State::Offline);

    fmt::print(stderr, "AudioControllerImpl::state_machine_loop(): shutdown\n");
}
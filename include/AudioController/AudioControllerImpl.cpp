//
// Created by Schizoneurax on 4/10/2025.
//

#include "AudioControllerImpl.hpp"
#include <chrono>
#include <fmt/base.h>
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
    if (std::shared_lock l(state_machine_mutex_); curr_state_ != State::Offline)
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
    machine_ssource_ = {};
    audio_ready_ = std::promise<void>{};
    curr_playback_id_ = 0;
    curr_active_button_.reset();
    audio_thread_ = std::jthread([this](const std::stop_token& stoken)
    {
        try { audio_event_loop(stoken); }
        catch (std::exception& e)
        {
            event_queue_->push(AudioErrorEvent{
                std::nullopt,
                "playAudioLoop: " + std::string(e.what()) + "\n"
            });
        }
    });
}

void AudioControllerImpl::start()
{
    if (audio_thread_.joinable() || state_machine_thread_.joinable())
    {
        throw std::runtime_error("AudioControllerImpl::start() failed: Instance running");
    }

    if (!event_queue_)
    {
        throw std::runtime_error("AudioControllerImpl::start() failed: No event_queue");
    }

    // Will only reach here if the current state is Offline, which means no other thread
    // is actively running. (Although state machine thread may haven't joined yet)
    start_audio_thread();

    state_machine_thread_ = std::jthread{
        [this](const std::stop_token& stoken)
        {
            // There is a level of indirection here.
            // Is there a way to let the created thread use the machine_ssource directly?
            std::stop_callback cb{
                stoken,
                [this]() { machine_ssource_.request_stop(); }
            };
            // state_machine_loop doesn't throw exception
            state_machine_loop(machine_ssource_.get_token());
        }
    };

    audio_ready_.get_future().wait();
}

void AudioControllerImpl::shutdown()
{
    if (state_machine_thread_.joinable())
    {
        if (std::shared_lock lock(state_machine_mutex_);
            curr_state_ != State::Offline)
        {
            // push_event<ShutdownEvent>();
            event_queue_->push(ShutdownEvent{});
        }
        // While we can't call join() while handling the ShutdownEvent,
        // we can join() here.
        state_machine_thread_.join();
    }
}

void AudioControllerImpl::play(const identifier_type id, const boost::filesystem::path& path)
{
    if (std::shared_lock l(state_machine_mutex_);
        curr_state_ != State::Offline)
    {
        event_queue_->push(PlayEvent{id, path});
    }
}

void AudioControllerImpl::pause(const identifier_type id)
{
    if (std::shared_lock lock(state_machine_mutex_);
        curr_state_ == State::Play)
    {
        event_queue_->push(PauseEvent{id});
    }
}

void AudioControllerImpl::resume(const identifier_type id)
{
    if (std::shared_lock lock(state_machine_mutex_);
        curr_state_ == State::Pause)
    {
        event_queue_->push(ResumeEvent{id});
    }
}

void AudioControllerImpl::stop(const identifier_type id)
{
    if (std::shared_lock l(state_machine_mutex_);
        curr_state_ != State::Offline && curr_state_ != State::Idle)
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
    // The playback_id let us tell whether we need to play a new audio.
    // Suppose the current state is Play, after a new PlayEvent has arrived the state is still Play.
    // Suppose the current state is Pause, after a new PlayEvent the state becomes Play.
    // In both two cases we have no way to know there's a new audio to play.
    // So here we just compare our playback_id with curr_playback_id_. The later will be updated
    // by the state change callback functions in case a new audio arrives.
    event_queue_->push(AudioReadyEvent{});
    uint64_t playback_id = 0;
    while (!stoken.stop_requested())
    {
        std::shared_lock lock(state_machine_mutex_);
        audio_condition_.wait(lock, stoken, [this]()
        {
            return curr_state_ == State::Play && curr_playback_id_;
        });
        if (stoken.stop_requested() || curr_state_ == State::Error) { break; }

        const auto audio_path = curr_audio_path_;
        if (curr_playback_id_)
        {
            playback_id = curr_playback_id_.value();
        }
        else
        {
            // push_event is a potentially blocking function
            lock.unlock();
            event_queue_->push(AudioErrorEvent{std::nullopt, "Unknown playback id"});
            break;
        }

        // For playback simulation only
        constexpr unsigned interval = 50;
        constexpr auto ratio = 1000 / interval;
        const auto duration = duration_.load() * ratio;

        lock.unlock();

        // Simulate audio playback
        for (unsigned i = 0; i < duration && !stoken.stop_requested(); i++)
        {
            // Play a new audio if playback id changes
            if (std::shared_lock l{state_machine_mutex_}; playback_id != curr_playback_id_)
            {
                fmt::print("AudioThread: playback id {} superseded by {}\n",
                           playback_id, curr_playback_id_.value_or(0));
                break;
            }

            // Actual simulation
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
                curr_state_ == State::Idle ||
                curr_state_ == State::Error ||
                stoken.stop_requested())
            {
                break;
            }
        }

        if (std::shared_lock state_lock(state_machine_mutex_);
            curr_state_ == State::Idle ||
            curr_state_ == State::Error ||
            stoken.stop_requested())
        {
            fmt::print("Audio stopped...\n");
        }
        // Here we've broken out of the loop while still have the same playback_id
        // We must have been playing the same audio to its end.
        // In real implementation there might be a more robust way?
        else if (playback_id == curr_playback_id_)
        {
            fmt::print("Audio finished naturally\n");
            state_lock.unlock();
            event_queue_->push(AudioFinishedEvent{playback_id});

            // Re-enter the loop only if state has been changed to idle
            if (state_lock.lock(); curr_state_ == State::Play)
            {
                audio_condition_.wait(state_lock, stoken, [this]()
                {
                    return curr_state_ != State::Play;
                });
            }
        }
    }
}

void AudioControllerImpl::reset_playback()
{
    assert(curr_state_ != State::Offline && curr_state_ != State::Error);
    // Reset all playback metadata for a new playback
    // Maybe it should be "populating all metadata for a new playback"
    // Definitely need to change in the future so I'll leave a TODO here.
    curr_state_ = State::Idle;
    curr_audio_path_ = "";
    duration_ = default_duration;
}

void AudioControllerImpl::play_callback(const PlayEvent& play_evt)
{
    // Play a new audio, irrespective of the current state,
    // except the machine has already shutdown, aka. in offline state.
    if (std::scoped_lock l(state_machine_mutex_);
        curr_state_ != State::Offline && curr_state_ != State::Error)
    {
        // Suppose to reset all metadata. Currently not much to do.
        // In real implementation, the PlayEvent struct should contain all necessary metadata
        // to populate. May need to change the "reset_playback" to "populate_metadata" or something
        reset_playback();
        fmt::print("Playing new audio...\n");
        // This is critical for the audio thread to distinguish a new audio from its current audio,
        ++curr_playback_id_;
        curr_state_ = State::Play;
        curr_active_button_ = play_evt.id;

        // PlayEvent will have more metadata in actual implementation and I should let
        // "reset_playback()" do the work instead of here.
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
        curr_state_ != State::Idle &&
        curr_state_ != State::Offline &&
        curr_state_ != State::Error)
    {
        // reset_playback(); Not necessary. A new PlayEvent will do that.
        curr_state_ = State::Idle;
        curr_active_button_.reset();

        // See comment in play_callback
        audio_condition_.notify_one();
    }
}

void AudioControllerImpl::audio_ready_callback(const AudioReadyEvent&)
{
    if (std::scoped_lock lock(state_machine_mutex_);
        curr_state_ == State::Offline || curr_state_ == State::Error)
    {
        curr_state_ = State::Idle;
        audio_ready_.set_value();
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
        // Audio thread goes to sleep after finishes the audio playback
        // so a notification is needed.
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
    // Make sure audio thread is not blocked after firing the AudioErrorEvent
    // Otherwise deadlock occurs.
    if (audio_thread_.joinable())
    {
        audio_thread_.join();
    }

    start_audio_thread();
}


void AudioControllerImpl::state_machine_loop(const std::stop_token& stoken) noexcept
{
    static const auto visit = Visitor{
        [this](const PlayEvent& evt) { play_callback(evt); },
        [this](const PauseEvent& evt) { pause_callback(evt); },
        [this](const ResumeEvent& evt) { resume_callback(evt); },
        [this](const StopEvent& evt) { stop_callback(evt); },
        [this](const AudioReadyEvent& evt) { audio_ready_callback(evt); },
        [this](const AudioFinishedEvent& evt) { audio_finished_callback(evt); },
        [this](const ShutdownEvent& evt) { shutdown_callback(evt); },
        [this](const AudioErrorEvent& evt) { error_callback(evt); },
    };

    while (!stoken.stop_requested())
    {
        try
        {
            auto evt = pop_event();
            std::visit(visit, evt);
            if (std::shared_lock l(state_machine_mutex_);
                curr_state_ == State::Offline)
            {
                break;
            }
        }
        catch (std::exception& e)
        {
            fmt::print("state_machine_loop: Exception occurred: {}\n", e.what());
            break;
        }
    }
}
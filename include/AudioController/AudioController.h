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
    void play(boost::filesystem::path path);
    void pause();
    void resume();
    void stop();

    void stateMachineLoop(const std::stop_token& stoken);

    template <auto MemPtr>
    const auto& atomic_getter() const
    {
        std::shared_lock lock(state_machine_mutex_);
        return this->*MemPtr;
    }

    template <auto MemPtr, typename T>
    void atomic_setter(T&& arg)
    {
        std::unique_lock lock(state_machine_mutex_);
        this->*MemPtr = std::forward<T>(arg);
    }

private:
    enum class State
    {
        Offline, Idle, Play, Pause
    };

    void state_change_epilogue();

    // To SDL2?
    void playAudio(const std::stop_token& stoken);
    // Todo: void pauseAudio() const;
    // Todo: void resumeAudio() const;

    std::jthread state_machine_thread_;
    mutable std::shared_mutex state_machine_mutex_;
    std::condition_variable_any state_machine_condition_;

    // Shared data
    State curr_state_{State::Offline};
    std::atomic_int duration_{10};
    std::atomic_bool audio_finished_naturally_{true};
    boost::filesystem::path curr_audio_path_{};
};


#endif //AUDIOCONTROLLER_H

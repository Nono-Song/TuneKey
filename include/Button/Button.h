//
// Created by Schizoneurax on 3/12/2025.
//
#pragma once
#include <any>
#include <string>
#include <cstdint>
#include <variant>
#include <boost/filesystem.hpp>

#include "AudioController.h"

class AudioController;
class ButtonManager;

class Button
{
public:
    using name_t = std::string;
    using uuid_t = uint32_t;
    using path_t = boost::filesystem::path;

    enum class SortKey
    {
        Name, UUID
    };

    template <typename U>
    using Proj = U(*)(const Button&);
    using ProjVariant = std::variant<
        Proj<name_t>,
        Proj<uuid_t>>;


    enum class ActionType
    {
        Play, Pause, Resume, Release, ModifyPath, ModifyName
    };

    Button() = delete;

    ~Button() noexcept;

    Button(uuid_t, name_t, ButtonManager*, AudioController*, const std::string& path = "");

    Button(const Button& other) = delete;

    Button(Button&& other) noexcept
        : name_{std::move(other.name_)},
          uuid_{other.uuid_},
          file_path_{std::move(other.file_path_)},
          audio_controller_{other.audio_controller_},
          button_manager_(other.button_manager_)
    {
    }

    Button& operator=(const Button& other) = delete;
    Button& operator=(Button&& other) = delete;

    /*****************************************************
     *                     Getter                       *
     ****************************************************/
    [[nodiscard]] name_t getName() const { return name_; }

    // The public method to obtain a projector given a key
    static ProjVariant Projector(const SortKey key)
    {
        // if constexpr (key == SortKey::Name)
        // {
        //     return createProjector<&Button::name_>();
        // }
        switch (key)
        {
        case SortKey::Name: return createProjector<&Button::name_>();
        case SortKey::UUID: return createProjector<&Button::uuid_>();
        default: throw std::invalid_argument("Invalid Key");
        }
    }

    // Execute an action with parameter
    void execute(const ActionType&, const std::any& param = {});

private:
    // Create a projector that project a button to one of its member data
    // according to the template variable MemberPtr
    template <auto MemberPtr>
    static auto createProjector() -> ProjVariant
    {
        return [](const Button& btn)
        {
            return btn.*MemberPtr;
        };
    }

    /*------------------Operations--------------------------*/
    // Play the audio file if file path is valid. Interface with AudioController
    // Update last_used_time
    void playAudio() const;

    // Pause the audio without releasing the resources. Only release the resources after a timeout
    void pauseAudio() const;
    void resumeAudio() const;

    // Stop and release the resources (file). Interface to FileController
    void release() const;

    // Modify the file path. Need to first release any hold resources
    template <typename T>
    void modifyFilePath(T&& arg)
    {
        file_path_ = std::forward<T>(arg);
    }

    // Use one single template function to do perfect forwarding
    template <typename Name>
    void modifyName(Name&& new_name)
    {
        name_ = std::forward<Name>(new_name);
    }

    // Todo: Time of creation
    // Todo: Time of last usage
    name_t name_;
    const uuid_t uuid_;
    path_t file_path_;
    AudioController* const audio_controller_;
    ButtonManager* const button_manager_;
};

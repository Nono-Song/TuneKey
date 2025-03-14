//
// Created by Schizoneurax on 3/12/2025.
//
#pragma once
#include <any>
#include <string>
#include <cstdint>
#include <functional>

#include <boost/filesystem.hpp>

class Button
{
    template <auto>
    struct always_false : std::false_type
    {
    };

public:
    using name_t = std::string;
    using uuid_t = uint32_t;
    using path_t = boost::filesystem::path;

    enum class SortKey
    {
        Name, UUID
    };

    template <typename U>
    using Proj = std::function<U(const Button&)>;
    using ProjVariant = std::variant<
        Proj<name_t>,
        Proj<uuid_t>>;


    enum class ActionType
    {
        Play, Pause, Release, ModifyPath, ModifyName
    };

    Button() = delete;

    ~Button();

    Button(uuid_t, name_t, std::string name = "");

    [[nodiscard]] name_t getName() const { return name_; }

    static ProjVariant Projector(const SortKey key)
    {
        switch (key)
        {
        case SortKey::Name: return createProjector<&Button::name_>();
        case SortKey::UUID: return createProjector<&Button::uuid_>();
        default: throw std::invalid_argument("Invalid Key");
        }
    }

    void execute(const ActionType&, const std::any&);

private:
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

    // Stop and release the resources (file). Interface to FileController
    void release();

    // Modify the file path. Need to first release any hold resources
    void modifyFilePath(std::string&&);

    // Use one single template function to do perfect forwarding
    template <typename Name>
    void modifyName(Name&&);

    // Todo: Time of creation
    // Todo: Time of last usage
    name_t name_;
    uuid_t uuid_;
    // Todo: Need a more specific way to represent file path
    path_t file_path_;
};

template <typename Name>
void Button::modifyName(Name&& new_name)
{
    name_ = std::forward<Name>(new_name);
}

//
// Created by Schizoneurax on 3/12/2025.
//
#pragma once
#include <string>
#include <variant>
#include <boost/filesystem.hpp>

template <typename T>
class EventQueue;

class Button
{
public:
    using name_type = std::string;
    using identifier_type = size_t;
    using filepath_type = boost::filesystem::path;

    /** Projector **/
    template <typename U>
    using Proj = const U&(*)(const Button&);
    using ProjVariant = std::variant<
        Proj<name_type>,
        Proj<identifier_type>,
        Proj<filepath_type>>;


    using SortKeyType = std::variant<identifier_type, name_type, filepath_type>;
    /** The public method to obtain a projector given a key **/
    template <typename Key>
        requires std::same_as<Key, Button::name_type> ||
        std::same_as<Key, Button::identifier_type> ||
        std::same_as<Key, Button::filepath_type>
    static ProjVariant Projector()
    {
        using U = Key;
        if constexpr (std::is_same_v<U, name_type>)
        {
            return createProjector<&Button::name_>();
        }
        else if constexpr (std::is_same_v<U, identifier_type>)
        {
            return createProjector<&Button::id_>();
        }
        else // if (std::is_same_v<U, filepath_type>)
        {
            return createProjector<&Button::file_path_>();
        }
    }

private:
    /** Create a projector that project a button to one of its member data
     * according to the template variable MemberPtr **/
    template <auto MemberPtr>
    static auto createProjector() -> ProjVariant
    {
        using member_type = std::invoke_result_t<decltype(MemberPtr), const Button&>;
        return [](const Button& btn) -> member_type
        {
            return std::invoke(MemberPtr, btn);
        };
    }

public:
    /** ButtonEvent **/
    enum class ActionType
    {
        Play, Pause, Resume, Release
    };

    struct ButtonEvent
    {
        identifier_type id;
        ActionType action;
    };

    using ButtonEventType = ButtonEvent;

    /** Ctor, Dtor and Copy Control **/
    Button() = delete;

    ~Button() noexcept;

    Button(const name_type&, EventQueue<ButtonEvent>* queue);
    Button(name_type, const std::string& path, EventQueue<ButtonEvent>* queue);

    Button(const Button& other) = delete;

    Button(Button&& other) noexcept
        : name_{std::move(other.name_)},
          id_{other.id_},
          file_path_{std::move(other.file_path_)}, event_queue_(other.event_queue_)
    {
    }

    Button& operator=(const Button& other) = delete;
    Button& operator=(Button&& other) = delete;

    /** Getter & Setter  **/
    [[nodiscard]] const name_type& getName() const { return name_; }
    [[nodiscard]] const identifier_type& getID() const { return id_; }
    [[nodiscard]] const filepath_type& getFilePath() const { return file_path_; }

    template <typename T>
    void modifyFilePath(T&& arg)
    {
        release();
        file_path_ = std::forward<T>(arg);
    }

    template <typename Name>
    void modifyName(Name&& new_name)
    {
        name_ = std::forward<Name>(new_name);
    }

private:
    /*------------------Operations--------------------------*/
    // Play the audio file if file path is valid. Interface with AudioController
    // Update last_used_time
    void playAudio() const;

    // Pause the audio without releasing the resources. Only release the resources after a timeout
    void pauseAudio() const;
    void resumeAudio() const;

    // Stop and release the resources (file). Interface to FileController
    void release() const;

    inline static std::atomic<size_t> next_id_{0};

    // Todo: Time of creation
    // Todo: Time of last usage
    name_type name_;
    const identifier_type id_;
    filepath_type file_path_;
    EventQueue<ButtonEventType>* event_queue_;
};

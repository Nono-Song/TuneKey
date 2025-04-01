//
// Created by Schizoneurax on 3/12/2025.
//
#pragma once
#include <EventQueue.hpp>
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
    struct PlayEvent;
    struct ResumeEvent;
    struct PauseEvent;
    struct StopEvent;
    using event_type = std::variant<PlayEvent,
                                    PauseEvent,
                                    ResumeEvent,
                                    StopEvent>;

    using event_queue = EventQueue<event_type>;

    struct PlayEvent
    {
        identifier_type id;
        filepath_type filepath;
    };

    struct PauseEvent
    {
        identifier_type id;
    };

    struct ResumeEvent
    {
        identifier_type id;
    };

    struct StopEvent
    {
        identifier_type id;
    };

    /** Ctor, Dtor and Copy Control **/
    Button(const name_type&, const identifier_type& id, EventQueue<event_type>* queue);
    Button(name_type, const identifier_type& id, const std::string& path, EventQueue<event_type>* queue);
    Button(Button&& other) noexcept;
    ~Button() noexcept;

    Button() = delete;
    Button(const Button& other) = delete;
    Button& operator=(const Button& other) = delete;
    Button& operator=(Button&& other) = delete;

    /** Projector **/
    template <typename U>
    using Proj = const U&(*)(const Button&);
    using ProjVariant = std::variant<
        Proj<name_type>,
        Proj<identifier_type>,
        Proj<filepath_type>>;

    // The public method to obtain a projector given a key
    template <typename Key>
        requires
        std::same_as<Key, Button::name_type> ||
        std::same_as<Key, Button::identifier_type> ||
        std::same_as<Key, Button::filepath_type>
    static ProjVariant Projector();

private:
    /** Create a projector that project a button to one of its member data
     * according to the template variable MemberPtr **/
    template <auto MemberPtr>
    static ProjVariant createProjector();

public:
    /** Getter & Setter  **/
    [[nodiscard]] const name_type& getName() const { return name_; }
    [[nodiscard]] const identifier_type& getID() const { return id_; }
    [[nodiscard]] const filepath_type& getFilePath() const { return file_path_; }
    void modify_name(const name_type& arg);
    void modify_filepath(const filepath_type& arg);

    template <typename Evt>
        requires std::is_same_v<Evt, Button::PlayEvent> ||
        std::is_same_v<Evt, Button::PauseEvent> ||
        std::is_same_v<Evt, Button::ResumeEvent> ||
        std::is_same_v<Evt, Button::StopEvent>
    void handleEvent() const;

private:
    template <typename Attr, typename T>
        requires std::is_convertible_v<Attr, Button::filepath_type> || std::is_convertible_v<Attr, Button::name_type>
    void modify_attribute(T&& arg);

    // Todo: Time of creation
    // Todo: Time of last usage
    name_type name_;
    const identifier_type id_;
    filepath_type file_path_;
    event_queue* event_queue_;
};

/*-------------------------------------------------------------------*
 *             Template Member function Implementation               *
 *-------------------------------------------------------------------*/
template <typename Attr, typename T>
    requires std::is_convertible_v<Attr, Button::filepath_type> ||
    std::is_convertible_v<Attr, Button::name_type>
void Button::modify_attribute(T&& arg)
{
    using U = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<U, name_type>)
    {
        name_ = std::forward<T>(arg);
    }
    else // if constexpr (std::is_same_v<U, filepath_type>)
    {
        file_path_ = std::forward<T>(arg);
    }
}

template <auto MemberPtr>
Button::ProjVariant Button::createProjector()
{
    using member_type = std::invoke_result_t<decltype(MemberPtr), const Button&>;
    return [](const Button& btn) -> member_type
    {
        return std::invoke(MemberPtr, btn);
    };
}

template <typename Key>
    requires std::same_as<Key, Button::name_type> ||
    std::same_as<Key, Button::identifier_type> ||
    std::same_as<Key, Button::filepath_type>
Button::ProjVariant Button::Projector()
{
    using U = Key;
    if constexpr (std::is_same_v<U, Button::name_type>)
    {
        return createProjector<&Button::name_>();
    }
    else if constexpr (std::is_same_v<U, Button::identifier_type>)
    {
        return createProjector<&Button::id_>();
    }
    else // if (std::is_same_v<U, filepath_type>)
    {
        return createProjector<&Button::file_path_>();
    }
}

template <typename Evt>
    requires std::is_same_v<Evt, Button::PlayEvent> ||
    std::is_same_v<Evt, Button::PauseEvent> ||
    std::is_same_v<Evt, Button::ResumeEvent> ||
    std::is_same_v<Evt, Button::StopEvent>
void Button::handleEvent() const
{
    if constexpr (std::is_same_v<Evt, PlayEvent>)
    {
        event_queue_->push(Evt{id_, file_path_});
    }
    else if constexpr (std::is_same_v<Evt, PauseEvent>)
    {
        event_queue_->push(Evt{id_});
    }
    else if constexpr (std::is_same_v<Evt, ResumeEvent>)
    {
        event_queue_->push(Evt{id_});
    }
    else // if constexpr (std::is_same_v<Evt, StopEvent>)
    {
        event_queue_->push(Evt{id_});
    }
}

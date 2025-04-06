//
// Created by Schizoneurax on 3/12/2025.
//
#pragma once
#include <EventQueue.hpp>
#include <string>
#include <variant>
#include <boost/filesystem.hpp>
#include <ButtonEvent.hpp>

template <typename T>
class EventQueue;

class Button
{
    using event_type = ButtonEvent;

public:
    using event_queue = EventQueue<event_type>;

    /** Ctor, Dtor and Copy Control **/
    Button(const name_type&, const identifier_type& id, event_queue* queue);
    Button(name_type, const identifier_type& id, filename_type path, event_queue* queue);
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
        Proj<filename_type>>;

    // The public method to obtain a projector given a key
    template <typename Key>
        requires
        std::same_as<Key, name_type> ||
        std::same_as<Key, identifier_type> ||
        std::same_as<Key, filename_type>
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
    [[nodiscard]] const filename_type& getFilePath() const { return file_path_; }
    void modify_name(const name_type& arg);
    void modify_filepath(const filename_type& arg);
    template <typename Attr, typename T>
        requires std::is_convertible_v<Attr, filename_type> || std::is_convertible_v<Attr, name_type>
    void modify_attribute(T&& arg);

    template <typename Evt>
        requires std::is_same_v<Evt, PlayEvent> ||
        std::is_same_v<Evt, PauseEvent> ||
        std::is_same_v<Evt, ResumeEvent> ||
        std::is_same_v<Evt, StopEvent>
    void handleEvent() const;

private:
    // Todo: Time of creation
    // Todo: Time of last usage
    name_type name_;
    const identifier_type id_;
    filename_type file_path_;
    event_queue* event_queue_;
};

/*-------------------------------------------------------------------*
 *             Template Member function Implementation               *
 *-------------------------------------------------------------------*/
template <typename Attr, typename T>
    requires std::is_convertible_v<Attr, filename_type> ||
    std::is_convertible_v<Attr, name_type>
void Button::modify_attribute(T&& arg)
{
    using U = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<U, name_type>)
    {
        name_ = std::forward<T>(arg);
    }
    else // if constexpr (std::is_same_v<U, filename_type>)
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
    requires std::same_as<Key, name_type> ||
    std::same_as<Key, identifier_type> ||
    std::same_as<Key, filename_type>
Button::ProjVariant Button::Projector()
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
    else // if (std::is_same_v<U, filename_type>)
    {
        return createProjector<&Button::file_path_>();
    }
}

template <typename Evt>
    requires std::is_same_v<Evt, PlayEvent> ||
    std::is_same_v<Evt, PauseEvent> ||
    std::is_same_v<Evt, ResumeEvent> ||
    std::is_same_v<Evt, StopEvent>
void Button::handleEvent() const
{
    if constexpr (std::is_same_v<Evt, PlayEvent>)
    {
        event_queue_->push(Evt{id_, file_path_});
    }
    else
    {
        event_queue_->push(Evt{id_});
    }
}

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


template <typename T>
concept ButtonAttr = requires()
{
    std::is_convertible_v<T, name_type> ||
    std::is_convertible_v<T, identifier_type> ||
    std::is_convertible_v<T, filename_type>;
};

template <typename Evt>
concept ButtonEvent = requires()
{
    std::same_as<Evt, PlayEvent> ||
    std::same_as<Evt, PauseEvent> ||
    std::same_as<Evt, ResumeEvent> ||
    std::same_as<Evt, StopEvent> ;
};

class Button
{
public:
    using event_type = std::variant<PlayEvent,
                                    PauseEvent, ResumeEvent,
                                    StopEvent, ShutdownEvent>;
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
    template <ButtonAttr Attr>
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

    // template <ButtonAttr Attr>
    // void modify_attribute(Attr&& arg);

    template <ButtonEvent Evt>
    void handleEvent() const;

private:
    event_queue* event_queue_;
    // Todo: Time of creation
    // Todo: Time of last usage
    name_type name_;
    const identifier_type id_;
    filename_type file_path_;
};

/*-------------------------------------------------------------------*
 *             Template Member function Implementation               *
 *-------------------------------------------------------------------*/
// template <ButtonAttr Attr>
// void Button::modify_attribute(Attr&& arg)
// {
//     using U = std::decay_t<decltype(arg)>;
//     if constexpr (std::is_convertible_v<U, name_type>)
//     {
//         name_ = std::forward<Attr>(arg);
//     }
//     else // if constexpr (std::is_same_v<U, filename_type>)
//     {
//         file_path_ = std::forward<Attr>(arg);
//     }
// }

template <auto MemberPtr>
Button::ProjVariant Button::createProjector()
{
    using member_type = std::invoke_result_t<decltype(MemberPtr), const Button&>;
    return [](const Button& btn) -> member_type
    {
        return std::invoke(MemberPtr, btn);
    };
}

template <ButtonAttr Attr>
Button::ProjVariant Button::Projector()
{
    using U = Attr;
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

template <ButtonEvent Evt>
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

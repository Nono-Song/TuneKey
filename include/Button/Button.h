//
// Created by Schizoneurax on 3/12/2025.
//
#pragma once
#include <EventQueue.hpp>
#include <string>
#include <variant>
#include <concepts>
#include <boost/filesystem.hpp>
#include <Event.hpp>

template <typename... T>
constexpr bool always_false = false;

template <typename T>
class EventQueue;

template <typename T>
concept Identifier = std::same_as<T, identifier_type>;

template <typename T>
concept NameType = std::same_as<T, name_type>;

template <typename T>
concept Filename = std::same_as<T, filename_type>;

template <typename T>
concept ButtonEvent =
    std::same_as<T, PlayEvent> ||
    std::same_as<T, PauseEvent> ||
    std::same_as<T, StopEvent> ||
    std::same_as<T, ResumeEvent>;


template <typename T>
concept ButtonAttr = NameType<T> || Filename<T> || Identifier<T>;

template <typename T>
concept ButtonAttrArg = std::assignable_from<name_type&, T> || std::assignable_from<filename_type&, T>;

class Button
{
    // Make friend with ButtonManager voids the meaning of my complex projector functions...
    // But maybe some other code will make uses of them?
    friend class ButtonManager;

public:
    using event_type = Event;
    using event_queue = EventQueue<event_type>;

    /** Ctor, Dtor and Copy Control **/
    Button(const name_type&, const identifier_type& id, std::unique_ptr<event_queue>& queue);
    Button(name_type name, const identifier_type& id, filename_type path, std::unique_ptr<event_queue>& queue);

    ~Button() noexcept;
    Button(Button&& other) noexcept;
    Button(const Button& other) = delete;
    Button& operator=(const Button& other) = delete;
    Button& operator=(Button&& other) = delete;

    /** Projector **/
    template <ButtonAttr U>
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

    template <ButtonAttr Attr, ButtonAttrArg S>
    void modify(S&& arg);

    template <ButtonEvent Evt>
    void handleEvent() const;

private:
    std::unique_ptr<event_queue>& event_queue_;
    // Todo: Time of creation
    // Todo: Time of last usage
    name_type name_;
    const identifier_type id_;
    filename_type file_path_;
};

/*-------------------------------------------------------------------*
 *             Template Member function Implementation               *
 *-------------------------------------------------------------------*/
template <ButtonAttr Attr, ButtonAttrArg S>
void Button::modify(S&& arg)
{
    using T = decltype(arg);
    if constexpr (std::is_same_v<Attr, name_type>)
    {
        name_ = std::forward<T>(arg);
    }
    else if constexpr (std::is_same_v<Attr, filename_type>)
    {
        file_path_ = std::forward<T>(arg);
    }
    else
    {
        static_assert(false, "Unhandled case in Button::modify. "
                      "Update if constexpr chain for concept ButtonAttr");
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
    else if constexpr (std::is_same_v<U, filename_type>)
    {
        return createProjector<&Button::file_path_>();
    }
    else
    {
        static_assert(always_false<U>,
                      "Unhandled case in Button::Projector: Update if constexpr chain for ButtonAttr!");
        throw std::logic_error("Unhandled case in Button::Projector");
    }
}

template <ButtonEvent Evt>
void Button::handleEvent() const
{
    if constexpr (std::is_same_v<Evt, PlayEvent>)
    {
        event_queue_->push(Evt{id_, file_path_});
    }
    else if constexpr (
        std::is_same_v<Evt, PauseEvent> ||
        std::is_same_v<Evt, ResumeEvent> ||
        std::is_same_v<Evt, StopEvent>)
    {
        event_queue_->push(Evt{id_});
    }
    else
    {
        static_assert(always_false<Evt>,
                      "Unhandled case in Button::handleEvent: "
                      "not a Button event or change in ButtonEvent concept");
    }
}

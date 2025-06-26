//
// Created by Schizoneurax on 3/12/2025.
//
#pragma once
#include <Typedefs.hpp>
#include <functional>
#include <stdexcept>

struct IAudioController;

class Button
{
public:
    using event_type = Event;
    /** Ctor, Dtor and Copy Control **/
    Button(const name_type&, identifier_type, IAudioController*);
    Button(name_type, identifier_type, filename_type, IAudioController*);

    Button(Button&& other) noexcept;
    virtual ~Button() noexcept;
    Button(const Button& other) = delete;
    Button& operator=(const Button& other) = delete;
    Button& operator=(Button&& other) = delete;

    // The public method to obtain a projector given a key
    template <ButtonAttr Attr>
    static ButtonProjectorVariant Projector();

private:
    /** Create a projector that project a button to one of its member data
     * according to the template variable MemberPtr **/
    template <auto MemberPtr>
    static ButtonProjectorVariant createProjector();

public:
    /** Getter & Setter  **/
    [[nodiscard]] const name_type& getName() const { return name_; }
    [[nodiscard]] const identifier_type& getID() const { return id_; }
    [[nodiscard]] const filename_type& getFilePath() const { return file_path_; }

    template <ModifiableAttr Attr, ButtonAttrArg S>
    void modify(S&& arg);

    virtual void interact() const = 0;

protected:
    template <ButtonEvent Evt>
    void handleEvent() const;

private:
    IAudioController* controller_;
    // Todo: Time of creation
    // Todo: Time of last usage
    name_type name_;
    const identifier_type id_;
    filename_type file_path_;
};

/*-------------------------------------------------------------------*
 *             Template Member function Implementation               *
 *-------------------------------------------------------------------*/
template <ModifiableAttr Attr, ButtonAttrArg S>
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
ButtonProjectorVariant Button::createProjector()
{
    // The `std::invoke_result_t` type trait calculates the resulting type we would get
    // by invoking the given member pointer (`MemberPtr`) on an object of a specific type.
    // The type we specify here (`const Button&`) is the key to our contract: it dictates
    // what kind of parameter our final lambda will accept.
    using member_type = std::invoke_result_t<decltype(MemberPtr), const Button&>;

    // We return a lambda that creates the "projection".
    // Its parameter type, `const Button&`, MUST match the type used in `std::invoke_result_t`
    // above. This consistency is what makes the type deduction work correctly.
    return [](const Button& btn) -> member_type
    {
        // std::invoke is a helper that can call member functions or access member
        // variables through pointers to them.
        return std::invoke(MemberPtr, btn);
    };
}

template <ButtonAttr Attr>
ButtonProjectorVariant Button::Projector()
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
        static_assert(false,
                      "Unhandled case in Button::Projector: Update if constexpr chain for ButtonAttr!");
        throw std::logic_error("Unhandled case in Button::Projector");
    }
}

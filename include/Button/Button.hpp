//
// Created by Schizoneurax on 3/12/2025.
//
#pragma once
#include <Typedefs.hpp>
#include <stdexcept>
#include <variant>

namespace TuneKey
{
    class Button;
    struct IAudioController;

    template<typename T> concept Identifier = std::same_as<T, identifier_type>;
    template<typename T> concept NameType = std::same_as<T, name_type>;
    template<typename T> concept FilenameType = std::same_as<T, filename_type>;
    template<typename T> concept TimestampType = std::same_as<T, timestamp_type>;


    template<typename T>
    concept ButtonAttr = NameType<T> || FilenameType<T> || Identifier<T> || TimestampType<T>;

    template<typename T>
    concept ModifiableAttr = NameType<T> || FilenameType<T>;
    template<typename T>
    concept ButtonAttrArg = std::assignable_from<name_type &, T> || std::assignable_from<filename_type &, T>;

    /** Projector **/
    template<ButtonAttr U>
    using ButtonProjector = const U&(*)(const Button &);
    using ButtonProjectorVariant = std::variant<
        ButtonProjector<name_type>,
        ButtonProjector<identifier_type>,
        ButtonProjector<filename_type>,
        ButtonProjector<timestamp_type> >;

    class Button
    {
    public:
        /** Ctor, Dtor and Copy Control **/
        Button(const name_type&, identifier_type);

        Button(name_type, identifier_type, filename_type);

        Button(Button&& other) noexcept;
        virtual ~Button() noexcept;
        Button(const Button& other) = delete;
        Button& operator=(const Button& other) = delete;
        Button& operator=(Button&& other) = delete;

        // The public method to obtain a projector given a key
        template<ButtonAttr Attr>
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

        virtual void interact(IAudioController* controller) const = 0;

    private:
        timestamp_type creation_time_;
        name_type name_;
        const identifier_type id_;
        filename_type file_path_;
    };

    /*-------------------------------------------------------------------*
     *             Template Member function Implementation               *
     *-------------------------------------------------------------------*/
    template<ModifiableAttr Attr, ButtonAttrArg S>
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

    template<auto MemberPtr>
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
}

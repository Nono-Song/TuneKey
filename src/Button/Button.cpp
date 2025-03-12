//
// Created by Schizoneurax on 3/11/2025.
//
#include "Button.h"
#include <stdexcept>


Button::Button(const uuid_t uuid, name_t name, std::string path)
    : name(std::move(name)), uuid(uuid), file_path(std::move(path))
{
};

std::unique_ptr<Button::Button_Cmp_t> Button::comparator(const SearchKey key)
{
    switch (key)
    {
    case SearchKey::NAME: return createComparator<&Button::name>();
    case SearchKey::UUID: return createComparator<&Button::uuid>();
    // Other cases to be added
    default: throw std::invalid_argument("invalid search key");
    }
}

template <auto MemberPtr>
std::unique_ptr<Button::Button_Cmp_t> Button::createComparator()
{
    return std::make_unique<Button_Cmp_t>([](const Button& b1, const Button& b2) -> bool
    {
        return b1.*MemberPtr < b2.*MemberPtr;
    });
}

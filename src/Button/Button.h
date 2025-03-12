//
// Created by Schizoneurax on 3/12/2025.
//
#pragma once
#include <string>
#include <cstdint>
#include <functional>
#include <memory>

class Button
{
public:
    using name_t = std::string;
    using uuid_t = uint32_t;
    using Button_Cmp_t = std::function<bool(const Button&, const Button&)>;

    enum class SearchKey
    {
        NAME, UUID
    };

    Button() = delete;

    Button(const uuid_t uuid, name_t name, std::string path = "");

    [[nodiscard]] name_t getName() const { return name; }

    static std::unique_ptr<Button_Cmp_t> comparator(const SearchKey key);

private:
    template <auto MemberPtr>
    static std::unique_ptr<Button_Cmp_t> createComparator();

    // Todo: Time of creation
    // Todo: Time of modification
    // Todo: Time of last usage
    name_t name;
    uuid_t uuid;
    // Todo: Need a more specific way to represent file path
    std::string file_path;
};

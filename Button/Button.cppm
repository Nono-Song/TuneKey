//
// Created by Schizoneurax on 3/11/2025.
//
module;
#include <cstdint>
#include <string>
export module Button;

export class Button
{
public:
    using name_t = std::string;
    using uuid_t = uint32_t;

    Button(const uuid_t uuid, const name_t& name): name(name), uuid(uuid)
    {
    };

    Button(const uuid_t uuid, name_t&& name): name(std::move(name)), uuid(uuid)
    {
    }

    [[nodiscard]] name_t get_name() const { return name; }

private:
    name_t name;
    uuid_t uuid;
};

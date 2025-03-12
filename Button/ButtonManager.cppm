//
// Created by Schizoneurax on 3/11/2025.
//
module;
#include <stdexcept>
#include <vector>
#include <unordered_map>
export module ButtonManager;

import Button;

export class ButtonManager
{
    using uuid_t = Button::uuid_t;
    using name_t = Button::name_t;

public:
    ButtonManager();
    ~ButtonManager();
    ButtonManager(const ButtonManager&) = delete;

    ButtonManager(ButtonManager&& other) noexcept: button_view(std::move(other.button_view)),
                                                   name_to_uuid(std::move(other.name_to_uuid)),
                                                   button_map(std::move(other.button_map))
    {
    }

    ButtonManager& operator=(const ButtonManager&) = delete;

    ButtonManager& operator=(ButtonManager&& other) noexcept
    {
        button_view = std::move(other.button_view);
        name_to_uuid = std::move(other.name_to_uuid);
        button_map = std::move(other.button_map);
        return *this;
    };

    void add_button(const name_t& name)
    {
        // Check name conflict
        if (name_to_uuid.contains(name))
        {
            throw std::invalid_argument("The name already exists");
        }
        const auto uuid = get_next_uuid();
        button_map.emplace(uuid, Button(uuid, name));
        name_to_uuid.emplace(name, uuid);
        button_view.emplace_back(uuid);
    }

    void delete_button(const uuid_t& uuid)
    {
        if (const auto nh = button_map.extract(uuid); !nh.empty())
        {
            const auto id = nh.key();
            const auto& btn = nh.mapped();

            erase(button_view, id);
            erase_if(name_to_uuid, [&btn](const std::pair<name_t, uuid_t>& p)
            {
                return p.first == btn.get_name();
            });
        }
    }

    [[nodiscard]] bool contains(const uuid_t& uuid) const
    {
        return button_map.contains(uuid);
    }

    [[nodiscard]] const std::vector<uuid_t>& get_view() const { return button_view; }

    void reorder(const std::vector<uuid_t>::difference_type& idx_from,
                 const std::vector<uuid_t>::difference_type& idx_to)
    {
        throw std::logic_error("Not implemented");
    }

private:
    static constexpr uint32_t MAX_NBUTTON = 100;
    uuid_t curr_uuid = 0;

    uuid_t get_next_uuid()
    {
        return ++curr_uuid;
    }

    std::vector<uuid_t> button_view;
    std::unordered_map<name_t, uuid_t> name_to_uuid;
    std::unordered_map<uuid_t, Button> button_map;
};

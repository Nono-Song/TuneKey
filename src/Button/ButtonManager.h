//
// Created by Schizoneurax on 3/12/2025.
//

#pragma once
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include "Button.h"

class ButtonManager
{
    using uuid_t = Button::uuid_t;
    using name_t = Button::name_t;

public:
    ButtonManager() = default;
    ~ButtonManager() = default;
    ButtonManager(const ButtonManager&) = delete;

    ButtonManager(ButtonManager&& other) noexcept;

    ButtonManager& operator=(const ButtonManager&) = delete;

    ButtonManager& operator=(ButtonManager&& other) noexcept;

    template <typename Name>
    void addButton(Name&& name)
    {
        if (name_to_uuid.contains(name))
        {
            throw std::invalid_argument("The name already exists");
        }
        updateUUID();
        const auto uuid = getUUID();
        button_map.emplace(uuid, Button(uuid, std::forward<Name>(name)));
        name_to_uuid.emplace(std::forward<Name>(name), uuid);
        button_view.emplace_back(uuid);
    }

    void deleteButton(const uuid_t& uuid);

    bool contains(const uuid_t& uuid) const;

    const std::vector<uuid_t>& getView() const;

    void reorder(const std::vector<uuid_t>::difference_type& idx_from,
                 const std::vector<uuid_t>::difference_type& idx_to);

    void sort(const std::unique_ptr<Button::Button_Cmp_t>& comp);

private:
    static constexpr uint32_t MAX_NBUTTON = 100;
    uuid_t curr_uuid = 0;

    uuid_t getUUID() const { return curr_uuid; }

    void updateUUID() { ++curr_uuid; }

    std::vector<uuid_t> button_view{};
    std::unordered_map<name_t, uuid_t> name_to_uuid{};
    std::unordered_map<uuid_t, Button> button_map{};
};

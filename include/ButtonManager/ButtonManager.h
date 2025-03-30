//
// Created by Schizoneurax on 3/12/2025.
//

#pragma once
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

    void addButton(const name_t& name);
    void deleteButton(const uuid_t& uuid);

    const std::vector<uuid_t>& getView() const;

    void reorder(const std::vector<uuid_t>::difference_type& idx_from,
                 const std::vector<uuid_t>::difference_type& idx_to);
    void sort(const std::unique_ptr<Button::Button_Cmp_t>& comp);

    void startEventLoop();

private:
    static constexpr uint32_t MAX_NBUTTON = 100;
    uuid_t curr_uuid = 0;

    [[nodiscard]] constexpr uuid_t getUUID() const { return curr_uuid; }

    inline void updateUUID() { ++curr_uuid; }

    std::vector<uuid_t> button_view{};
    std::unordered_map<name_t, uuid_t> name_to_uuid{};
    std::unordered_map<uuid_t, Button> button_map{};
};

//
// Created by Schizoneurax on 3/12/2025.
//

#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include "Button.h"
#include <optional>

class AudioController;

class ButtonManager
{
    using uuid_t = Button::uuid_t;
    using name_t = Button::name_t;

public:
    ButtonManager();

    ~ButtonManager() = default;

    ButtonManager(const ButtonManager&) = delete;

    ButtonManager(ButtonManager&& other) noexcept;

    ButtonManager& operator=(const ButtonManager&) = delete;

    ButtonManager& operator=(ButtonManager&& other) noexcept;

    Button& operator[](const uuid_t& uuid);

    uuid_t addButton(const name_t& name);

    void deleteButton(const uuid_t& uuid);

    [[nodiscard]] const std::vector<uuid_t>& getView() const;

    void reorder(const std::vector<uuid_t>::difference_type& idx_from,
                 const std::vector<uuid_t>::difference_type& idx_to);

    void sortBy(Button::SortKey, bool reverse = false);

    [[nodiscard]] const std::optional<uuid_t>& getActiveButton() const;
    void setActiveButton(const uuid_t& uuid);
    void clearActiveButton();

private
:
    static constexpr uint32_t MAX_NBUTTON = 100;
    uuid_t curr_uuid = 0;

    [[nodiscard]] constexpr uuid_t getUUID() const
    {
        return curr_uuid;
    }

    void updateUUID()
    {
        ++curr_uuid;
    }

    std::unique_ptr<AudioController> audio_controller{};
    std::unordered_map<uuid_t, Button> button_map{};
    std::vector<uuid_t> button_view{};
    std::unordered_map<name_t, uuid_t> name_to_uuid{};
    std::optional<uuid_t> active_button_{};
};

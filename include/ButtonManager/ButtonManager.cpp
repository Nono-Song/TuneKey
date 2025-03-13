//
// Created by Schizoneurax on 3/11/2025.
//

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include "ButtonManager.h"

using uuid_t = Button::uuid_t;
using name_t = Button::name_t;
using Button_Cmp_t = Button::Button_Cmp_t;

ButtonManager::ButtonManager(ButtonManager &&other) noexcept : button_view(std::move(other.button_view)),
                                                               name_to_uuid(std::move(other.name_to_uuid)),
                                                               button_map(std::move(other.button_map)) {
}

ButtonManager &ButtonManager::operator=(ButtonManager &&other) noexcept {
    button_view = std::move(other.button_view);
    name_to_uuid = std::move(other.name_to_uuid);
    button_map = std::move(other.button_map);
    return *this;
};

void ButtonManager::addButton(const name_t &name) {
    if (name_to_uuid.contains(name)) {
        throw std::invalid_argument("The name already exists");
    }
    updateUUID();

    button_map.emplace(getUUID(), Button(getUUID(), name));
    name_to_uuid.emplace(name, getUUID());
    button_view.emplace_back(getUUID());
}


void ButtonManager::deleteButton(const uuid_t &uuid) {
    if (const auto nh = button_map.extract(uuid); !nh.empty()) {
        const auto &id = nh.key();
        const auto &btn = nh.mapped();

        erase(button_view, id);
        erase_if(name_to_uuid, [&btn](const auto &p) {
            return p.first == btn.getName();
        });
    }
}

[[nodiscard]] const std::vector<uuid_t> &ButtonManager::getView() const { return button_view; }

void ButtonManager::reorder(const std::vector<uuid_t>::difference_type &idx_from,
                            const std::vector<uuid_t>::difference_type &idx_to) {
    throw std::logic_error("Not implemented");
}

void ButtonManager::startEventLoop() {
    throw std::logic_error("Not implemented");
}

void ButtonManager::addEvent(Event &&event) {
    event_queue_.Push(std::move(event));
}

void ButtonManager::sort(const std::unique_ptr<Button::Button_Cmp_t> &comp) {
    std::sort(button_view.begin(), button_view.end(),
              [this, &comp](const uuid_t &u1, const uuid_t &u2) {
                  return (*comp)(button_map.at(u1), button_map.at(u2));
              });
}

//
// Created by Schizoneurax on 3/11/2025.
//

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include "ButtonManager.h"

ButtonManager::ButtonManager(ButtonManager&& other) noexcept : button_view(std::move(other.button_view)),
                                                               name_to_uuid(std::move(other.name_to_uuid)),
                                                               button_map(std::move(other.button_map))
{
}

ButtonManager& ButtonManager::operator=(ButtonManager&& other) noexcept
{
    button_view = std::move(other.button_view);
    name_to_uuid = std::move(other.name_to_uuid);
    button_map = std::move(other.button_map);
    return *this;
}

const Button &ButtonManager::operator[](const uuid_t &uuid) const {
    return button_map.at(uuid);
};

void ButtonManager::addButton(const name_t& name)
{
    if (name_to_uuid.contains(name))
    {
        throw std::invalid_argument("The name already exists");
    }
    updateUUID();

    button_map.emplace(getUUID(), Button(getUUID(), name));
    name_to_uuid.emplace(name, getUUID());
    button_view.emplace_back(getUUID());
}


void ButtonManager::deleteButton(const uuid_t& uuid)
{
    if (const auto nh = button_map.extract(uuid); !nh.empty())
    {
        const auto& id = nh.key();
        const auto& btn = nh.mapped();

        erase(button_view, id);
        erase_if(name_to_uuid, [&btn](const auto& p)
        {
            return p.first == btn.getName();
        });
    }
}

void ButtonManager::sortBy(const Button::SortKey key)
{
    // Why I can't use auto& proj here?
    std::visit([this](const auto& proj)
               {
                   std::ranges::sort(button_view,
                                     std::less{},
                                     [this, &proj](const uuid_t id)
                                     {
                                         return proj(button_map.at(id));
                                     }
                   );
               }
               ,
               Button::Projector(key)
    );
}

const std::vector<ButtonManager::uuid_t> &ButtonManager::getView() const { return button_view; }

void ButtonManager::reorder(const std::vector<uuid_t>::difference_type& idx_from,
                            const std::vector<uuid_t>::difference_type& idx_to)
{
    throw std::logic_error("Not implemented");
}

void ButtonManager::startEventLoop()
{
    throw std::logic_error("Not implemented");
}

void ButtonManager::addEvent(Event&& event)
{
    event_queue_.Push(std::move(event));
}

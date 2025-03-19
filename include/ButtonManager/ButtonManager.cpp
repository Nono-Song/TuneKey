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

const Button& ButtonManager::operator[](const uuid_t& uuid) const
{
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

void ButtonManager::sortBy(const Button::SortKey key, bool reverse)
{
    // Why I can't use auto& proj here?
    std::visit([this, reverse](const auto& proj)
               {
                   auto cmp = [reverse](const auto& x, const auto& y)
                   {
                       return reverse ? std::greater<>{}(x, y) : std::less<>{}(x, y);
                   };
                   auto projector = [this, &proj](const uuid_t id)
                   {
                       return proj(button_map.at(id));
                   };
                   std::ranges::sort(button_view, cmp, projector
                   );
               }
               ,
               Button::Projector(key)
    );
}

const std::vector<ButtonManager::uuid_t>& ButtonManager::getView() const { return button_view; }

void ButtonManager::reorder(const std::vector<uuid_t>::difference_type& idx_from,
                            const std::vector<uuid_t>::difference_type& idx_to)
{
    if (idx_from == idx_to)
    {
        return;
    }

    const auto id = button_view.at(idx_from);
    erase(button_view, id);
    button_view.insert(button_view.cbegin() + idx_to, id);
}

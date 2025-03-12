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
};

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

[[nodiscard]] bool ButtonManager::contains(const uuid_t& uuid) const
{
    return button_map.contains(uuid);
}

[[nodiscard]] const std::vector<uuid_t>& ButtonManager::getView() const { return button_view; }

/*void reorder(const std::vector<uuid_t>::difference_type& idx_from,
             const std::vector<uuid_t>::difference_type& idx_to)
{
    throw std::logic_error("Not implemented");
}*/

void ButtonManager::sort(const std::unique_ptr<Button::Button_Cmp_t>& comp)
{
    std::sort(button_view.begin(), button_view.end(),
              [this, &comp](const uuid_t& u1, const uuid_t& u2)
              {
                  return (*comp)(button_map.at(u1), button_map.at(u2));
              });
}

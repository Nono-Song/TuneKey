//
// Created by Schizoneurax on 3/11/2025.
//

#include <stdexcept>
#include <vector>
#include <unordered_map>
#include "ButtonManager.h"
#include "AudioController.h"

ButtonManager::ButtonManager() noexcept = default;

Button& ButtonManager::operator[](const identifier_type& id)
{
    return button_map.at(id);
};

ButtonManager::identifier_type ButtonManager::addButton(const name_type& name, const filepath_type& filepath)
{
    if (name_to_uuid.contains(name))
    {
        throw std::invalid_argument("The name already exists");
    }

    auto new_id = next_id_++;
    auto createButton = [this, &name, new_id, &filepath]()
    {
        return Button(name, new_id, filepath, event_queue_);
    };

    button_map.emplace(new_id, createButton());
    name_to_uuid.emplace(name, new_id);
    button_view.emplace_back(new_id);

    return new_id;
}


void ButtonManager::deleteButton(const identifier_type& target_id)
{
    if (const auto nh = button_map.extract(target_id); !nh.empty())
    {
        const auto& id = nh.key();
        const auto& btn = nh.mapped();

        if (getActiveButton() == id)
        {
            audio_controller->stop();
            clearActiveButton();
        }

        erase(button_view, id);
        erase_if(name_to_uuid, [&btn](const auto& p)
        {
            return p.first == btn.getName();
        });
    }
}

const std::vector<ButtonManager::identifier_type>& ButtonManager::getView() const { return button_view; }

void ButtonManager::reorder(const std::vector<identifier_type>::difference_type& idx_from,
                            const std::vector<identifier_type>::difference_type& idx_to)
{
    if (idx_from == idx_to)
    {
        return;
    }

    const auto id = button_view.at(idx_from);
    erase(button_view, id);
    button_view.insert(button_view.cbegin() + idx_to, id);
}

void ButtonManager::modify_button_name(const identifier_type& id, const name_type& name)
{
    modify_button_attr<name_type>(id, name);
}

void ButtonManager::modify_button_filepath(const identifier_type& id, const filepath_type& path)
{
    if (getActiveButton() == id)
    {
        audio_controller->stop();
    }

    modify_button_attr<filepath_type>(id, path);
}


const std::optional<ButtonManager::identifier_type>& ButtonManager::getActiveButton() const
{
    return active_button_;
}

void ButtonManager::setActiveButton(const identifier_type& target_id)
{
    active_button_ = target_id;
}

void ButtonManager::clearActiveButton()
{
    active_button_.reset();
}

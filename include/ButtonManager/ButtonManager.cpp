//
// Created by Schizoneurax on 3/11/2025.
//

#include "ButtonManager.hpp"
#include "AudioController.hpp"
#include "SpecialButtons.hpp"

ButtonManager::ButtonManager(): audio_controller(AudioController::create())
{
}

ButtonManager::~ButtonManager() = default;

const Button& ButtonManager::operator[](const identifier_type id) const
{
    return *button_map.at(id);
}

identifier_type ButtonManager::addButton(name_type name, filename_type filepath)
{
    auto new_id = next_id_++;
    auto createButton = [this, &name, new_id, &filepath]()-> button_ptr
    {
        if (name_to_uuid.contains(name))
        {
            name += "_2";
        }
        return std::make_unique<PlayButton>(std::move(name),
                                            new_id,
                                            std::move(filepath),
                                            audio_controller.get());
    };

    button_map.emplace(new_id, createButton());
    name_to_uuid.emplace(button_map.at(new_id)->getName(), new_id);
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
            audio_controller->stop(id);
        }

        const auto cnt1 = erase(button_view, id);
        const auto cnt2 = erase_if(name_to_uuid, [&btn](const auto& p)
        {
            return p.first == btn->getName();
        });

        if (cnt1 != cnt2 || cnt1 != 1)
        {
            throw std::runtime_error("Underlying data corrupted");
        }
    }
}

const std::vector<identifier_type>& ButtonManager::getView() const { return button_view; }

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


const std::optional<identifier_type>& ButtonManager::getActiveButton() const
{
    return active_button_;
}

void ButtonManager::setActiveButton(const identifier_type& id)
{
    active_button_ = id;
}

void ButtonManager::clearActiveButton()
{
    active_button_.reset();
}

template <typename Name>
    requires std::assignable_from<name_type&, Name>
void ButtonManager::modify_button_name(const identifier_type id, Name&& name)
{
    using T = decltype(name);

    auto& btn = button_map.at(id);
    auto nh = name_to_uuid.extract(btn->getName());

    if (!name_to_uuid.contains(name) || name_to_uuid.at(name) == id)
    {
        btn->modify<name_type>(std::forward<T>(name));
    }
    else
    {
        btn->modify<name_type>(name + "_2");
    }

    nh.key() = btn->getName();

    name_to_uuid.insert(std::move(nh));
}

template <typename Filename>
    requires std::assignable_from<filename_type&, Filename>
void ButtonManager::modify_button_filepath(const identifier_type id, Filename&& path)
{
    using T = decltype(path);
    auto& btn = button_map.at(id);
    if (getActiveButton() == id)
    {
        audio_controller->stop(id);
    }

    btn->modify<filename_type>(std::forward<T>(path));
}

void ButtonManager::modify_name(const identifier_type id, name_type&& name)
{
    modify_button_name(id, std::move(name));
}

void ButtonManager::modify_name(const identifier_type id, const name_type& name)
{
    modify_button_name(id, name);
}

void ButtonManager::modify_filename(const identifier_type id, filename_type&& filename)
{
    modify_button_filepath(id, std::move(filename));
}

void ButtonManager::modify_filename(const identifier_type id, const filename_type& filename)
{
    modify_button_filepath(id, filename);
}

void ButtonManager::modify_filename(const identifier_type id, const char* filename)
{
    modify_button_filepath(id, filename);
}




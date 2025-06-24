//
// Created by Schizoneurax on 3/11/2025.
//

#include "ButtonManager.hpp"
#include "IAudioController.hpp"
#include "SpecialButtons.hpp"

ButtonManager::ButtonManager(std::unique_ptr<IAudioController>&& controller)
: audio_controller(std::move(controller))
{
}

ButtonManager::~ButtonManager() = default;

void ButtonManager::start()
{
    audio_controller->start();
}

void ButtonManager::shutdown()
{
    audio_controller->shutdown();
}

const Button& ButtonManager::operator[](const identifier_type id) const
{
    return *button_map.at(id);
}

identifier_type ButtonManager::addButton(name_type name, filename_type filepath)
{
    if (size(button_view) == MAX_NBUTTON)
    {
        throw std::out_of_range("Exceeded maximum allowable number of buttons");
    }

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

std::optional<identifier_type> ButtonManager::getActiveButton() const
{
    return audio_controller->active_button();
}

template <typename Name>
    requires std::assignable_from<name_type&, Name>
void ButtonManager::modify_button_name(const identifier_type id, Name&& new_name)
{
    using T = decltype(new_name);

    auto& btn = button_map.at(id);
    if (btn->getName() == new_name) { return; }

    auto nh = name_to_uuid.extract(btn->getName());

    if (!name_to_uuid.contains(new_name))
    {
        btn->modify<name_type>(std::forward<T>(new_name));
    }
    else
    {
        btn->modify<name_type>(new_name + "_2");
    }

    nh.key() = btn->getName();

    name_to_uuid.insert(std::move(nh));
}

template <typename Filename>
    requires std::assignable_from<filename_type&, Filename>
void ButtonManager::modify_button_filepath(const identifier_type id, Filename&& new_path)
{
    using T = decltype(new_path);
    auto& btn = button_map.at(id);
    if (getActiveButton() == id)
    {
        audio_controller->stop(id);
    }

    btn->modify<filename_type>(std::forward<T>(new_path));
}

void ButtonManager::modify_name(const identifier_type id, name_type&& new_name)
{
    modify_button_name(id, std::move(new_name));
}

void ButtonManager::modify_name(const identifier_type id, const name_type& new_name)
{
    modify_button_name(id, new_name);
}

void ButtonManager::modify_filename(const identifier_type id, filename_type&& new_filename)
{
    modify_button_filepath(id, std::move(new_filename));
}

void ButtonManager::modify_filename(const identifier_type id, const filename_type& new_filename)
{
    modify_button_filepath(id, new_filename);
}

void ButtonManager::modify_filename(const identifier_type id, const char* new_filename)
{
    modify_button_filepath(id, new_filename);
}




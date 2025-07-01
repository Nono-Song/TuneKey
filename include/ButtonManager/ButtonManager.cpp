//
// Created by Schizoneurax on 3/11/2025.
//

#include "ButtonManager.hpp"
#include "IAudioController.hpp"
#include "AudioButtons.hpp"
#include <algorithm>

using namespace TuneKey;

ButtonManager::ButtonManager(std::unique_ptr<IAudioController>&& controller)
: audio_controller_(std::move(controller))
{
}

ButtonManager::~ButtonManager() = default;

void ButtonManager::start()
{
    audio_controller_->start();
}

void ButtonManager::shutdown()
{
    audio_controller_->shutdown();
}

const Button& ButtonManager::operator[](const identifier_type id) const
{
    return *button_map_.at(id);
}

identifier_type ButtonManager::add_button(name_type name, filename_type filepath)
{
    if (size(button_view_) == MAX_NBUTTON)
    {
        throw std::out_of_range("Exceeded maximum allowable number of buttons");
    }

    auto new_id = next_id_++;
    auto createButton = [this, &name, new_id, &filepath]()-> button_ptr
    {
        if (name_to_uuid_.contains(name))
        {
            name += "_2";
        }
        return std::make_unique<PlayButton>(std::move(name),
                                            new_id,
                                            std::move(filepath));
    };

    button_map_.emplace(new_id, createButton());
    name_to_uuid_.emplace(button_map_.at(new_id)->getName(), new_id);
    button_view_.emplace_back(new_id);

    return new_id;
}


void ButtonManager::delete_button(const identifier_type& target_id)
{
    if (const auto nh = button_map_.extract(target_id); !nh.empty())
    {
        const auto& id = nh.key();
        const auto& btn = nh.mapped();
        if (getActiveButton() == id)
        {
            audio_controller_->stop(id);
        }

        const auto cnt1 = erase(button_view_, id);
        const auto cnt2 = erase_if(name_to_uuid_, [&btn](const auto& p)
        {
            return p.first == btn->getName();
        });

        if (cnt1 != cnt2 || cnt1 != 1)
        {
            throw std::runtime_error("Underlying data corrupted");
        }
    }
}

void ButtonManager::press_button(const identifier_type& id)
{
    button_map_.at(id)->interact(audio_controller_.get());
}

void ButtonManager::reorder(const std::vector<identifier_type>::difference_type& idx_from,
                            const std::vector<identifier_type>::difference_type& idx_to)
{
    if (idx_from == idx_to)
    {
        return;
    }

    const auto id = button_view_.at(idx_from);
    erase(button_view_, id);
    button_view_.insert(button_view_.cbegin() + idx_to, id);
}

std::optional<identifier_type> ButtonManager::getActiveButton() const
{
    return audio_controller_->active_button();
}

template <typename  Key>
void ButtonManager::sort(const std::function<bool(const Key&, const Key&)>& comp)
{
    const auto projector_variant = Button::Projector<Key>();
    const auto& proj = std::get<ButtonProjector<Key>>(projector_variant);

    auto projector = [this, proj](const identifier_type id)
    {
        return proj(*button_map_.at(id));
    };
    std::ranges::sort(button_view_, comp, projector);
}

template void ButtonManager::sort<identifier_type>(const std::function<bool(const identifier_type&, const identifier_type&)>& cmp);
template void ButtonManager::sort<filename_type>(const std::function<bool(const filename_type&, const filename_type&)>& cmp);
template void ButtonManager::sort<name_type>(const std::function<bool(const name_type&, const name_type&)>& cmp);
// template void ButtonManager::sort<timestamp_type>(const std::function<bool(const timestamp_type&, const timestamp_type&)>& cmp);


template <typename Name>
    requires std::assignable_from<name_type&, Name>
void ButtonManager::modify_button_name(const identifier_type id, Name&& new_name)
{
    using T = decltype(new_name);

    auto& btn = button_map_.at(id);
    if (btn->getName() == new_name) { return; }

    auto nh = name_to_uuid_.extract(btn->getName());

    if (!name_to_uuid_.contains(new_name))
    {
        btn->modify<name_type>(std::forward<T>(new_name));
    }
    else
    {
        btn->modify<name_type>(new_name + "_2");
    }

    nh.key() = btn->getName();

    name_to_uuid_.insert(std::move(nh));
}

template <typename Filename>
    requires std::assignable_from<filename_type&, Filename>
void ButtonManager::modify_button_filepath(const identifier_type id, Filename&& new_path)
{
    using T = decltype(new_path);
    auto& btn = button_map_.at(id);
    if (getActiveButton() == id)
    {
        audio_controller_->stop(id);
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




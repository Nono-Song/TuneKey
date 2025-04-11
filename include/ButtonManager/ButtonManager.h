//
// Created by Schizoneurax on 3/12/2025.
//

#pragma once
#include <vector>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <Button.hpp>

#include "AudioController.hpp"

class AudioController;
template <typename T>
class EventQueue;

class ButtonManager
{
public:
    using identifier_type = Button::identifier_type;
    using name_type = Button::name_type;
    using filepath_type = Button::filepath_type;
    using button_type = Button;
    using event_type = Button::event_type;
    using event_queue = Button::event_queue;

    ButtonManager() noexcept;
    ~ButtonManager() = default;

    ButtonManager(const ButtonManager&) = delete;
    ButtonManager(ButtonManager&&) = delete;
    ButtonManager& operator=(const ButtonManager&) = delete;
    ButtonManager& operator=(ButtonManager&&) = delete;

    button_type& operator[](const identifier_type& id);

    identifier_type addButton(const name_type& name, const filepath_type& filepath = "");
    void deleteButton(const identifier_type& id);

    [[nodiscard]] const std::vector<identifier_type>& getView() const;

    void reorder(const std::vector<identifier_type>::difference_type& idx_from,
                 const std::vector<identifier_type>::difference_type& idx_to);

    void modify_button_name(const identifier_type& id, const name_type& name);
    void modify_button_filepath(const identifier_type& id, const filepath_type& path);

    template <typename Key = Button::identifier_type>
        requires std::is_same_v<Key, Button::identifier_type> ||
        std::is_convertible_v<Key, Button::name_type> ||
        std::is_convertible_v<Key, Button::filepath_type>
    void sortViewReverse()
    {
        sortView<Key>(true);
    }

    template <typename Key = Button::identifier_type>
        requires std::is_same_v<Key, Button::identifier_type> ||
        std::is_convertible_v<Key, Button::name_type> ||
        std::is_convertible_v<Key, Button::filepath_type>
    void sortView(bool reverse = false)
    {
        std::visit([this, reverse](const auto& proj)
                   {
                       auto cmp = [reverse](const auto& x, const auto& y)
                       {
                           return reverse ? std::greater<>{}(x, y) : std::less<>{}(x, y);
                       };
                       auto projector = [this, &proj](const identifier_type id)
                       {
                           return proj(button_map.at(id));
                       };
                       std::ranges::sort(button_view, cmp, projector);
                   }
                   ,
                   Button::Projector<Key>()
        );
    }

    [[nodiscard]] const std::optional<identifier_type>& getActiveButton() const;
    void setActiveButton(const identifier_type& id);
    void clearActiveButton();

private:
    template <typename Attr>
        requires std::is_convertible_v<Attr, name_type> ||
        std::is_convertible_v<Attr, filepath_type>
    constexpr void modify_button_attr(const identifier_type id, auto&& arg)
    {
        button_map.at(id).modify_attribute<Attr>(std::forward<decltype(arg)>(arg));
    }

    void event_loop(const std::stop_token& stoken)
    {
        while (!stoken.stop_requested())
        {
            auto evt = event_queue_->pop();
            std::visit([this]<typename Evt>(Evt&& arg)
            {
                using U = std::decay_t<Evt>;
                if constexpr (std::is_same_v<U, Button::PlayEvent>)
                {
                    setActiveButton(arg.id);
                    audio_controller->play(arg.filepath);
                }
                else if constexpr (std::is_same_v<U, Button::PauseEvent>)
                {
                    audio_controller->pause();
                }
                else if constexpr (std::is_same_v<U, Button::ResumeEvent>)
                {
                    audio_controller->resume();
                }
                else // if constexpr (std::is_same_v<U, Button::StopEvent>)
                {
                    clearActiveButton();
                    audio_controller->stop();
                }
            }, evt);
        }
    }

    static constexpr size_t MAX_NBUTTON = 100;
    identifier_type next_id_ = 0;
    std::unique_ptr<AudioController> audio_controller{new AudioController()};
    event_queue* event_queue_;
    std::unordered_map<identifier_type, Button> button_map{};
    std::vector<identifier_type> button_view{};
    std::unordered_map<name_type, identifier_type> name_to_uuid{};
    std::optional<identifier_type> active_button_{};
};

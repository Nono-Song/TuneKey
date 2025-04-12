//
// Created by Schizoneurax on 3/12/2025.
//

#pragma once
#include <vector>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <SpecialButtons.hpp>


struct AudioController;
class Button;

class ButtonManager
{
public:
    using button_type = Button;
    using button_ptr = std::unique_ptr<button_type>;

    ButtonManager();
    ~ButtonManager();

    ButtonManager(const ButtonManager&) = delete;
    ButtonManager(ButtonManager&&) noexcept = default;
    ButtonManager& operator=(const ButtonManager&) = delete;
    ButtonManager& operator=(ButtonManager&&) = delete;

    void start();
    void shutdown();

    const button_type& operator[](identifier_type id) const;

    identifier_type addButton(name_type name, filename_type filepath = "");
    void deleteButton(const identifier_type& target_id);

    [[nodiscard]] const std::vector<identifier_type>& getView() const;

    void reorder(const std::vector<identifier_type>::difference_type& idx_from,
                 const std::vector<identifier_type>::difference_type& idx_to);


    void modify_name(identifier_type id, name_type&& name);
    void modify_name(identifier_type id, const name_type& name);
    void modify_filename(identifier_type id, filename_type&& filename);
    void modify_filename(identifier_type id, const filename_type& filename);
    void modify_filename(identifier_type id, const std::string& filename);
    void modify_filename(identifier_type id, std::string&& filename);
    void modify_filename(identifier_type id, const char* filename);

    template <ButtonAttr Key = identifier_type>
    void sortViewReverse() { sortView<Key>(true); }

    template <ButtonAttr Key = identifier_type>
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
                           return proj(*button_map.at(id));
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
    template <typename Name>
        requires std::assignable_from<name_type&, Name>
    void modify_button_name(identifier_type id, Name&& name);

    template <typename Filename>
        requires std::assignable_from<filename_type&, Filename>
    void modify_button_filepath(identifier_type id, Filename&& path);

    static constexpr size_t MAX_NBUTTON = 100;

    identifier_type next_id_ = 0;
    std::unique_ptr<AudioController> audio_controller;

    std::unordered_map<identifier_type, button_ptr> button_map{};
    std::vector<identifier_type> button_view{};
    std::unordered_map<name_type, identifier_type> name_to_uuid{};
    std::optional<identifier_type> active_button_{};
};

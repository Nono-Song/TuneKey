//
// Created by Schizoneurax on 3/12/2025.
//

#pragma once
#include <vector>
#include <unordered_map>
#include <optional>
#include <memory>
#include <functional>
#include <Typedefs.hpp>


struct IAudioController;
class Button;

class ButtonManager
{
public:
    using button_type = Button;
    using button_ptr = std::unique_ptr<button_type>;

    explicit ButtonManager(std::unique_ptr<IAudioController>&& controller);
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

    [[nodiscard]] const std::vector<identifier_type>& getView() const { return button_view; }

    void reorder(const std::vector<identifier_type>::difference_type& idx_from,
                 const std::vector<identifier_type>::difference_type& idx_to);


    void modify_name(identifier_type id, name_type&& new_name);
    void modify_name(identifier_type id, const name_type& new_name);
    void modify_filename(identifier_type id, filename_type&& new_filename);
    void modify_filename(identifier_type id, const filename_type& new_filename);
    void modify_filename(identifier_type id, const char* new_filename);

    template <ButtonAttr Key>
    void sortView(const std::function<bool(const Key&, const Key&)>&);

    [[nodiscard]] std::optional<identifier_type> getActiveButton() const;

private:
    template <typename Name>
        requires std::assignable_from<name_type&, Name>
    void modify_button_name(identifier_type id, Name&& new_name);

    template <typename Filename>
        requires std::assignable_from<filename_type&, Filename>
    void modify_button_filepath(identifier_type id, Filename&& new_path);

    static constexpr size_t MAX_NBUTTON = 100;

    identifier_type next_id_ = 0;
    std::unique_ptr<IAudioController> audio_controller;

    std::unordered_map<identifier_type, button_ptr> button_map{};
    std::vector<identifier_type> button_view{};
    std::unordered_map<name_type, identifier_type> name_to_uuid{};
};

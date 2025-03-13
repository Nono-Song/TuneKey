//
// Created by Schizoneurax on 3/12/2025.
//
#pragma once
#include <any>
#include <string>
#include <cstdint>
#include <functional>
#include <memory>

#include <boost/filesystem.hpp>

class Button {
public:
    using name_t = std::string;
    using uuid_t = uint32_t;
    using path_t = boost::filesystem::path;
    using Button_Cmp_t = std::function<bool(const Button &, const Button &)>;

    enum class SearchKey {
        Name, UUID
    };

    enum class ActionType {
        Play, Pause, Release, ModifyPath, ModifyName
    };

    Button() = delete;

    ~Button();

    Button(uuid_t, name_t, std::string name = "");

    [[nodiscard]] name_t getName() const { return name_; }

    static std::unique_ptr<Button_Cmp_t> comparator(SearchKey);

    void execute(const ActionType &, const std::any &);

private:
    template<auto MemberPtr>
    static std::unique_ptr<Button_Cmp_t> createComparator();

    /*------------------Operations--------------------------*/
    // Play the audio file if file path is valid. Interface with AudioController
    // Update last_used_time
    void playAudio() const;

    // Pause the audio without releasing the resources. Only release the resources after a timeout
    void pauseAudio() const;

    // Stop and release the resources (file). Interface to FileController
    void release();

    // Modify the file path. Need to first release any hold resources
    void modifyFilePath(std::string &&);

    // Use one single template function to do perfect forwarding
    template<typename Name>
    void modifyName(Name &&new_name) {
        name_ = std::forward<Name>(new_name);
    }

    // Todo: Time of creation
    // Todo: Time of last usage
    name_t name_;
    uuid_t uuid_;
    // Todo: Need a more specific way to represent file path
    path_t file_path_;
};

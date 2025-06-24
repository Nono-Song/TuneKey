#include <string>
#include <fmt/base.h>
#include "ButtonManager.hpp"
#include <algorithm>
#include <thread>
#include <cassert>
#include <iostream>
#include "IAudioController.hpp"
#include "AudioControllerFactory.hpp"
#include "SpecialButtons.hpp"

using namespace std::chrono_literals;
void test_projector(const Button& button, const Button::ProjVariant& v)
{
    std::visit([&button]<typename T>(T&& arg)
    {
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, Button::Proj<identifier_type>>)
        {
            assert(arg(button) == button.getID());
        }
        else if constexpr (std::is_same_v<U, Button::Proj<name_type>>)
        {
            assert(arg(button) == button.getName());
        }
        else if constexpr (std::is_same_v<U, Button::Proj<filename_type>>)
        {
            assert(arg(button) == button.getFilePath());
        }
        else
        {
            assert(false);
        }
    }, v);
}

template <typename Test>
void test_event(const Button::event_type& v)
{
    std::visit([]<typename T>(T&&)
    {
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, Test>)
        {
            assert(true);
        }
        else
        {
            assert(false);
        }
    }, v);
}

void bm_test_worker()
{
    ButtonManager bm(AudioControllerFactory::createAudioController());
    const auto id1 = bm.addButton("5", "1");
    const auto id2 = bm.addButton("4", "3");
    const auto id3 = bm.addButton("3", "5");
    const auto id4 = bm.addButton("2", "4");
    const auto id5 = bm.addButton("1", "2");

    auto& view = bm.getView();

    bm.sortView<name_type>(std::less<>());
    auto v1 = std::vector{id5, id4, id3, id2, id1};
    assert(view == v1);

    bm.sortView<filename_type>(std::less<>());
    auto v2 = std::vector{id1, id5, id2, id4, id3};
    assert(view == v2);

    bm.sortView(std::less<>());
    auto v3 = std::vector{id1, id2, id3, id4, id5};
    assert(view == v3);

    bm.sortView<name_type>(std::greater<>());
    std::ranges::reverse(v1);
    assert(view == v1);

    bm.sortView<filename_type>(std::greater<>());
    std::ranges::reverse(v2);
    assert(view == v2);

    bm.sortView<identifier_type>(std::greater<>());
    std::ranges::reverse(v3);
    assert(view == v3);


    const auto id6 = bm.addButton("1");
    assert(bm[id6].getName() == "1_2");


    // Modifier Tests
    const name_type new_name1 = "new_name1";
    bm.modify_name(id3, new_name1);
    assert(bm[id3].getName() == new_name1);
    bm.modify_name(id3, "another_name");
    assert(bm[id3].getName() == "another_name");

    const filename_type new_filepath1 = "new_filepath1.txt";
    bm.modify_filename(id2, new_filepath1);
    assert(bm[id2].getFilePath() == new_filepath1);

    const std::string new_filepath2 = "123";
    bm.modify_filename(id2, new_filepath2);
    assert(bm[id2].getFilePath() == new_filepath2);

    bm.modify_filename(id1, std::string{"123"});
    assert(bm[id1].getFilePath() == "123");

    bm.modify_filename(id2, "another_filename");
    assert(bm[id2].getFilePath() == "another_filename");

    bm[id2].interact();
    bm[id3].interact();
    bm.start();
    bm[id4].interact();

    assert(bm.getActiveButton() == id4);
    std::this_thread::sleep_for(3s);
    assert(bm.getActiveButton() == id4);
    bm.modify_filename(id4, "yet_another_filename");

    std::this_thread::sleep_for(12s);
    bm.shutdown();

}

struct MockAudioController final : IAudioController
{
    void start() override
    {
    }

    void shutdown() override
    {
    }

    void play(identifier_type id, const filename_type& path) override
    {
        fmt::println("{}: {}", id, path.string());
    }

    void stop(identifier_type) override
    {
    }

    void resume(identifier_type) override
    {
    }

    void pause(identifier_type) override
    {
    }

    [[nodiscard]] std::optional<identifier_type> active_button() const override { return std::nullopt; }
};

void button_test_worker()
{
    const std::unique_ptr<IAudioController> controller = AudioControllerFactory::createAudioController();
    controller->start();

    PlayButton button("test1", 0, controller.get());
    PauseButton pause("test2", 0, controller.get());
    ResumeButton resume("test3", 0, controller.get());
    StopButton stop("test4", 0, controller.get());

    assert(button.getID() == 0);
    assert(button.getName() == "test1");
    assert(button.getFilePath().empty());

    button.modify<filename_type>("testpath.txt");
    assert(button.getFilePath().string() == "testpath.txt");

    button.modify<name_type>("name");
    assert(button.getName() == "name");

    test_projector(button, Button::Projector<identifier_type>());
    test_projector(button, Button::Projector<name_type>());
    test_projector(button, Button::Projector<filename_type>());

    button.interact();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    button.interact();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    pause.interact();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    resume.interact();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    button.interact();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    stop.interact();
    std::this_thread::sleep_for(std::chrono::seconds(3));

    controller->shutdown();
    controller->start();
    button.interact();
    std::this_thread::sleep_for(std::chrono::seconds(15));
    controller->shutdown();
}

void testAudioController()
{
    const auto controller = AudioControllerFactory::createAudioController();
    controller->start();

    std::string input;
    identifier_type id{0};
    while (std::cin >> input)
    {
        if (input == "play")
        {
            std::cin >> id;
        }
        fmt::print("Input: {}\n", input);

        try
        {
            if (input == "start")
            {
                controller->start();
            }
            if (input == "exit")
            {
                return;
            }
            if (input == "stop")
            {
                controller->stop(id);
            }

            if (input == "shutdown")
            {
                controller->shutdown();
            }

            if (input == "play")
            {
                controller->play(id, "");
            }

            if (input == "pause")
            {
                controller->pause(id);
            }

            if (input == "resume")
            {
                controller->resume(id);
            }
        }
        catch (std::exception& e)
        {
            fmt::print("Toplevel: {}\n", e.what());
        }
    }
}

int main()
{
    try
    {
        // testAudioController();
        // button_test_worker();
        bm_test_worker();
    }
    catch (std::exception& e)
    {
        fmt::print("Error: {}\n", e.what());
    }
    return 0;
}

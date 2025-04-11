#include <string>
#include <fmt/base.h>
#include <iostream>
#include "AudioController.hpp"
#include "SpecialButtons.hpp"

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

struct MockAudioController : public AudioController
{
    void start() override
    {
    }

    void shutdown() override
    {
    }

    void play(identifier_type id, const boost::filesystem::path& path) override
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

    std::optional<identifier_type> active_button() const override { return std::nullopt; }
};

void button_test_worker()
{
    const std::unique_ptr<AudioController> controller = AudioController::create();
    controller->start();

    PlayButton button("test1", 0, controller.get());
    PauseButton pause("test2", 0, controller.get());
    ResumeButton resume("test3", 0, controller.get());
    StopButton stop("test4", 0, controller.get());

    assert(button.getID() == 0);
    assert(button.getName() == "test1");
    assert(button.getFilePath().empty());

    button.modify<filename_type>("testpath.txt");
    assert(button.getFilePath() == "testpath.txt");

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
    const auto controller = AudioController::create();
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
    // testAudioController();
    // button_test_worker();
    return 0;
}

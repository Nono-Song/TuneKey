#include <string>
#include <fmt/base.h>
#include <iostream>
#include "AudioController.hpp"

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
    testAudioController();
    return 0;
}

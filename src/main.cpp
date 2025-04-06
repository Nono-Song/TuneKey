#include <iostream>

#include <fmt/core.h>
#include <iostream>
#include <string>
#include <AudioController.h>

int main()
{
    AudioController controller{};
    controller.start();
    std::string input;
    while (std::cin >> input)
    {
        fmt::print("Input: {}\n", input);

        try
        {
            if (input == "start")
            {
                controller.start();
            }
            if (input == "exit")
            {
                return 0;
            }
            if (input == "stop")
            {
                controller.stop();
            }

            if (input == "shutdown")
            {
                controller.shutdown();
            }

            if (input == "play")
            {
                controller.play("");
            }

            if (input == "pause")
            {
                controller.pause();
            }

            if (input == "resume")
            {
                controller.resume();
            }
        }
        catch (std::exception& e)
        {
            fmt::print("Toplevel: {}\n", e.what());
        }
    }

    return 0;
}

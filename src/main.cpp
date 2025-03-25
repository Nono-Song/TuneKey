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

        if (input == "exit")
        {
            return 0;
        }
        if (input == "stop")
        {
            controller.stop();
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

    return 0;
}

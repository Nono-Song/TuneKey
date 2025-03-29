#include <iostream>

#include <fmt/base.h>
#include <string>
#include <SDL3/SDL.h>

#include "ButtonManager.h"

int main()
{
    ButtonManager bm{};
    auto id = bm.addButton("b1");
    std::string input;
    while (std::cin >> input)
    {
        // fmt::print("Input: {}\n", input);
        SDL_Log("Input: %s\n", input.c_str());
        if (input == "exit")
        {
            bm[id].execute(Button::ActionType::Release);
            return 0;
        }
        if (input == "stop")
        {
            bm[id].execute(Button::ActionType::Release);
        }

        if (input == "play")
        {
            bm[id].execute(Button::ActionType::Play, "path1");
        }

        if (input == "pause")
        {
            bm[id].execute(Button::ActionType::Pause);
        }

        if (input == "resume")
        {
            bm[id].execute(Button::ActionType::Resume);
        }
    }

    return 0;
}

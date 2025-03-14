#include <iostream>

#include <fmt/core.h>
#include "ButtonManager.h"

int main()
{
    ButtonManager button_manager;

    button_manager.addButton("b5");
    button_manager.addButton("b4");
    button_manager.addButton("b3");
    button_manager.addButton("b2");
    button_manager.addButton("b1");

    std::string opt;
    std::cin >> opt;
    if (opt == "name")
    {
        button_manager.sortBy(Button::SortKey::Name);
    }
    else if (opt == "uuid")
    {
        button_manager.sortBy(Button::SortKey::UUID);
    }
    else
    {
        fmt::print("Wrong option.\n");
        return 0;
    }


    for (const auto& x : button_manager.getView())
    {
        fmt::print("{}, ", x);
    }
    return 0;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.

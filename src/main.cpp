#include <fmt/core.h>
#include "Button/ButtonManager.h"

int main()
{
    ButtonManager button_manager;

    button_manager.addButton("b5");
    button_manager.addButton("b4");
    button_manager.addButton("b3");
    button_manager.addButton("b2");
    button_manager.addButton("b1");

    button_manager.sort(Button::comparator(Button::SearchKey::NAME));

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


#include <string>
#include <SDL3/SDL.h>
#include <Button.h>
#include <EventQueue.hpp>

void test_projector(const Button& button, const Button::ProjVariant& v)
{
    std::visit([&button]<typename T>(T&& arg)
    {
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, Button::Proj<Button::identifier_type>>)
        {
            assert(arg(button) == button.getID());
        }
        else if constexpr (std::is_same_v<U, Button::Proj<Button::name_type>>)
        {
            assert(arg(button) == button.getName());
        }
        else
        {
            assert(arg(button) == button.getFilePath());
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

void worker(const std::stop_token& stoken)
{
    Button::event_queue event_queue(stoken);
    Button button("test1", &event_queue);

    assert(button.getID() == 0);
    assert(button.getName() == "test1");
    assert(button.getFilePath() == "");

    button.modifyFilePath("testpath.txt");
    assert(button.getFilePath() == "testpath.txt");

    button.modifyName("name");
    assert(button.getName() == "name");

    test_projector(button, Button::Projector<Button::identifier_type>());
    test_projector(button, Button::Projector<Button::name_type>());
    test_projector(button, Button::Projector<Button::filepath_type>());

    button.handleEvent<Button::PlayEvent>();
    test_event<Button::PlayEvent>(event_queue.pop());
    button.handleEvent<Button::PlayEvent>();
    button.handleEvent<Button::PauseEvent>();
    test_event<Button::PlayEvent>(event_queue.pop());
    test_event<Button::PauseEvent>(event_queue.pop());
    button.handleEvent<Button::PlayEvent>();
    button.handleEvent<Button::ResumeEvent>();
    test_event<Button::PlayEvent>(event_queue.pop());
    test_event<Button::ResumeEvent>(event_queue.pop());
}

int main()
{
    auto t = std::jthread(worker);
    t.join();
    return 0;
}

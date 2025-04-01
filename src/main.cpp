#include <string>
#include <SDL3/SDL.h>
#include <fmt/base.h>
#include <Button.h>
#include <ButtonManager.h>
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

void button_test_worker(const std::stop_token& stoken)
{
    Button::event_queue event_queue(stoken);
    Button button("test1", 0, &event_queue);

    assert(button.getID() == 0);
    assert(button.getName() == "test1");
    assert(button.getFilePath() == "");

    button.modify_filepath("testpath.txt");
    assert(button.getFilePath() == "testpath.txt");

    button.modify_name("name");
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

void bm_test_worker(const std::stop_token& stoken)
{
    ButtonManager::event_queue event_queue(stoken);
    ButtonManager bm;
    const auto id1 = bm.addButton("5", "1");
    const auto id2 = bm.addButton("4", "3");
    const auto id3 = bm.addButton("3", "5");
    const auto id4 = bm.addButton("2", "4");
    const auto id5 = bm.addButton("1", "2");

    auto& view = bm.getView();

    bm.sortView<Button::name_type>();
    auto v1 = std::vector<Button::identifier_type>{id5, id4, id3, id2, id1};
    assert(view == v1);

    bm.sortView<Button::filepath_type>();
    auto v2 = std::vector<Button::identifier_type>{id1, id5, id2, id4, id3};
    assert(view == v2);

    bm.sortView();
    auto v3 = std::vector<Button::identifier_type>{id1, id2, id3, id4, id5};
    assert(view == v3);

    bm.sortViewReverse<Button::name_type>();
    std::ranges::reverse(v1);
    assert(view == v1);

    bm.sortViewReverse<Button::filepath_type>();
    std::ranges::reverse(v2);
    assert(view == v2);

    bm.sortViewReverse<Button::identifier_type>();
    std::ranges::reverse(v3);
    assert(view == v3);

    try
    {
        bm.addButton("1");
        assert(false);
    }
    catch (std::invalid_argument&)
    {
    }
    catch (...)
    {
        assert(false);
    }

    // Modifier Tests
    const Button::name_type new_name1 = "new_name1";
    bm.modify_button_name(id3, new_name1);
    assert(bm[id3].getName() == new_name1);

    const Button::filepath_type new_filepath1 = "new_filepath1";
    bm.modify_button_filepath(id2, new_filepath1);
    assert(bm[id2].getFilePath() == new_filepath1);
}

int main()
{
    auto t = std::jthread(button_test_worker);
    t.join();
    t = std::jthread(bm_test_worker);
    t.join();
    return 0;
}

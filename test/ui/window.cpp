#include <stdafx.h>

#include <catch.hpp>
#include <ui/window.h>

SCENARIO("Window")
{
	using namespace mark;
	using namespace ui;
	GIVEN("An empty window")
	{
		ui::window window({});
		WHEN("We call mutable children on it")
		{
			let result = window.children();
			THEN("It should return an empty vector")
			{
				REQUIRE(result.empty());
			}
		}
		WHEN("We call front() on it")
		{
			let result = window.front();
			THEN("It should return an empty optional")
			{
				REQUIRE(!result.has_value());
			}
		}
		WHEN("We call back() on it")
		{
			let result = window.back();
			THEN("It should return an empty optional")
			{
				REQUIRE(!result.has_value());
			}
		}
	}
	GIVEN("A window with another window within it")
	{
		ui::window window({});
		auto child_window = std::make_unique<ui::window>(ui::window::info{});
		let& child_window_ref = *child_window;
		window.append(move(child_window));
		WHEN("We call front() on it")
		{
			let result = window.front();
			THEN("It should return an optional reference pointing to the child "
				 "window")
			{
				REQUIRE(result.has_value());
				REQUIRE(&*result == &child_window_ref);
			}
		}
		WHEN("We call back() on it")
		{
			let result = window.back();
			THEN("It should return an optional reference pointing to the child "
				 "window")
			{
				REQUIRE(result.has_value());
				REQUIRE(&*result == &child_window_ref);
			}
		}
		WHEN("We call mutable children on it")
		{
			let result = window.children();
			THEN("It should return a vector with one element in it")
			{
				REQUIRE(result.size() == 1);
			}
			THEN("The only element in the vector should be the child window")
			{
				REQUIRE(&result.front().get() == &child_window_ref);
			}
		}
		WHEN("We call immutable children on it")
		{
			let result = static_cast<const ui::window&>(window).children();
			THEN("It should return a vector with one element in it")
			{
				REQUIRE(result.size() == 1);
			}
			THEN("The only element in the vector should be the child window")
			{
				REQUIRE(&result.front().get() == &child_window_ref);
			}
		}
		WHEN("We call clear() on it")
		{
			window.clear();
			THEN("The window should be empty")
			{
				REQUIRE(window.children().empty() == true);
			}
		}
		WHEN("We remove the window from the window")
		{
			let result = window.remove(window.children().front().get());
			THEN("The window should be empty")
			{
				REQUIRE(window.children().empty() == true);
			}
			THEN("Remove should return a unique_ptr holding the removed child")
			{
				REQUIRE(result.get() == &child_window_ref);
			}
		}
		WHEN("We append another window into the container")
		{
			auto second_child =
				std::make_unique<ui::window>(ui::window::info{});
			let& second_child_ref = *second_child;
			window.append(move(second_child));
			THEN("The window children count should be two")
			{
				REQUIRE(window.children().size() == 2);
			}
			THEN("The first child should still be the original child window")
			{
				REQUIRE(&window.children().front().get() == &child_window_ref);
			}
			THEN("The second child should be the newly inserted window")
			{
				REQUIRE(&window.children().back().get() == &second_child_ref);
			}
			THEN("front() should return a reference to the first child")
			{
				REQUIRE(&*window.front() == &child_window_ref);
			}
			THEN("back() should return a reference to the second child")
			{
				REQUIRE(&*window.back() == &second_child_ref);
			}
		}
		WHEN("We insert another window before the original child")
		{
			auto second_child =
				std::make_unique<ui::window>(ui::window::info{});
			let& second_child_ref = *second_child;
			window.insert(*window.front(), move(second_child));
			THEN("The window children count should be two")
			{
				REQUIRE(window.children().size() == 2);
			}
			THEN("The first child should be the newly inserted child")
			{
				REQUIRE(&window.children().front().get() == &second_child_ref);
			}
			THEN("The second child should be the original child")
			{
				REQUIRE(&window.children().back().get() == &child_window_ref);
			}
			THEN(
				"front() should return a reference to the newly inserted child")
			{
				REQUIRE(&*window.front() == &second_child_ref);
			}
			THEN("back() should return a reference to the original child")
			{
				REQUIRE(&*window.back() == &child_window_ref);
			}
		}
	}
	GIVEN("A window with two other windows within it")
	{
		ui::window window({});
		window.append(std::make_unique<ui::window>(ui::window::info{}));
		window.append(std::make_unique<ui::window>(ui::window::info{}));
		let children = window.children();
		REQUIRE(children.size() == 2);
		WHEN("We remove the first child")
		{
			(void)window.remove(children.front());
			THEN("window children count should drop to one")
			{
				REQUIRE(window.children().size() == 1);
			}
			THEN("remaining window should be the second window")
			{
				REQUIRE(&*window.front() == &children.back().get());
			}
		}
		WHEN("We remove the second child")
		{
			(void)window.remove(children.back());
			THEN("window children count should drop to one")
			{
				REQUIRE(window.children().size() == 1);
			}
			THEN("remaining window should be the second window")
			{
				REQUIRE(&*window.front() == &children.front().get());
			}
		}
		WHEN("We add another window between the first and the second")
		{
			auto middle_child_ptr =
				std::make_unique<ui::window>(ui::window::info{});
			let& middle_child = *middle_child_ptr;
			window.insert(children.back(), move(middle_child_ptr));
			THEN("window children count should increase to three")
			{
				REQUIRE(window.children().size() == 3);
			}
			THEN("original first child should still be the first child")
			{
				REQUIRE(&*window.front() == &children.front().get());
			}
			THEN("middle child should be the newly inserted window")
			{
				REQUIRE(&window.children()[1].get() == &middle_child);
			}
			THEN("last child should still be the last child")
			{
				REQUIRE(&*window.back() == &children.back().get());
			}
		}
	}
	GIVEN("A window with three other windows within it")
	{
		ui::window window({});
		window.append(std::make_unique<ui::window>(ui::window::info{}));
		window.append(std::make_unique<ui::window>(ui::window::info{}));
		window.append(std::make_unique<ui::window>(ui::window::info{}));
		let children = window.children();
		WHEN("We remove the middle child") {
			let& middle_child = window.children()[1].get();
			let result = window.remove(middle_child);
			THEN("result should not be empty") {
				REQUIRE(result);
			}
			THEN("result should be equal to the middle child")
			{
				REQUIRE(result.get() == &middle_child);
			}
			THEN("child count should drop to 2") {
				REQUIRE(window.children().size() == 2);
			}
		}
	}
}

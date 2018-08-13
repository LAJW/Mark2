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
	}
	GIVEN("A window with another window within it")
	{
		ui::window window({});
		auto child_window = std::make_unique<ui::window>(ui::window::info{});
		let& child_window_ref = *child_window;
		window.insert(move(child_window));
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
	}
}

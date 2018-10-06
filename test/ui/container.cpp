#include <catch.hpp>
#include <interface/ui.h>
#include <module/cargo.h>
#include <random.h>
#include <resource/manager.h>
#include <stdafx.h>
#include <ui/action/drop_into_container.h>
#include <ui/action/grab_from_container.h>
#include <ui/action/recycle.h>
#include <ui/action/release_grabbed.h>
#include <ui/container.h>
#include <ui/event.h>
#include <ui/item_button.h>
#include <update_context.h>

namespace mark {

class ui_stub final : public interface::ui
{
public:
	explicit ui_stub(std::unique_ptr<interface::item> grabbed)
		: m_grabbed(move(grabbed))
	{}
	[[nodiscard]] optional<const interface::item&> grabbed() const
		noexcept override
	{
		if (m_grabbed) {
			return *m_grabbed;
		} else if (m_grabbed2) {
			return *m_grabbed2;
		} else {
			return {};
		}
	}
	void grabbed(std::unique_ptr<interface::item> grabbed) noexcept
	{
		m_grabbed = move(grabbed);
	}
	void grabbed(interface::item& grabbed) noexcept { m_grabbed2 = grabbed; }
	[[nodiscard]] bool in_recycler(const mark::interface::item& item) const
		noexcept override
	{
		return false;
	}

private:
	std::unique_ptr<interface::item> m_grabbed;
	optional<interface::item&> m_grabbed2;
};

SCENARIO("UI Container Element")
{
	ui_stub ui(nullptr);
	resource::manager_stub rm;
	random_stub random;
	module::cargo cargo(rm, random, YAML::Node());
	GIVEN("A container element bound to an empty cargo container")
	{
		ui::container container([&] {
			ui::container::info _;
			_.container = cargo;
			_.rm = rm;
			_.ui = ui;
			return _;
		}());
		WHEN("We do nothing")
		{
			THEN("cargo reference should be correctly set")
			{
				REQUIRE(&container.cargo() == &cargo);
			}
			THEN("Window should have no children")
			{
				REQUIRE(container.children().empty());
			}
		}
		WHEN(
			"We insert a new element to the bound cargo module and call update")
		{
			REQUIRE(success(cargo.attach(
				{ 0, 0 },
				std::make_unique<module::cargo>(rm, random, YAML::Node()))));
			update_context context(rm, random);
			context.dt = .016;
			container.update(context);
			THEN("Container should gain a new item button after calling update")
			{
				REQUIRE(container.children().size() == 1);
			}
			THEN("That button should be an instance of item_button")
			{
				let first_child = container.front();
				let& item_button =
					dynamic_cast<const ui::item_button*>(&*first_child);
				REQUIRE(item_button != nullptr);
			}
			THEN("That instance button should point to the only element in "
				 "our cargo module")
			{
				let first_child = container.front();
				let& item_button =
					dynamic_cast<const ui::item_button*>(&*first_child);
				REQUIRE(&item_button->item() == &*cargo.at({ 0, 0 }));
			}
		}
		WHEN("We call pos() on the UI container()")
		{
			THEN("it should default to { 0, 0 }")
			{
				REQUIRE(container.pos() == vi32());
			}
		}
		WHEN("We call size on it")
		{
			let size = container.size();
			THEN("It should return 16x6 grid size (16x4 and the header)")
			{
				REQUIRE(size.x == 16 * int(module::size));
				REQUIRE(size.y == 6 * int(module::size));
			}
		}
		WHEN("We click on an empty space in the container when UI holds a "
			 "grabbed item")
		{
			ui.grabbed(
				std::make_unique<module::cargo>(rm, random, YAML::Node()));
			ui::event event;
			// Top offset 16px (container border)
			// 2 blocks to the right and 1 block to the middle to place cursor
			// in the middle of the box
			event.absolute_cursor = { 16, 32 + 16 };
			event.cursor = { 16, 32 + 16 };
			event.shift = false;
			let actions = container.click(event);
			THEN("Container should return 'handled'") { REQUIRE(actions); }
			THEN("An action for placing the item at a specified position "
				 "should be returned")
			{
				REQUIRE(actions->size() == 1);
				let action =
					dynamic_cast<const ui::action::drop_into_container*>(
						actions->front().get());
				REQUIRE(&action->container() == &cargo);
				REQUIRE(action->pos().x == 0);
				REQUIRE(action->pos().y == 0);
			}
		}
		WHEN("We click on the empty space outside of the container"
			 "grabbed item")
		{
			ui::event event;
			// Top offset 16px (container border)
			// 2 blocks to the right and 1 block to the middle to place cursor
			// in the middle of the box
			event.absolute_cursor = { 16, 32 + 16 * 5 };
			event.cursor = { 16, 32 + 16 * 5 };
			event.shift = false;
			let actions = container.click(event);
			THEN("Click should return 'unhandled'")
			{
				REQUIRE(!actions);
			}
		}
	}
	GIVEN("A container element bound to a cargo container with a single child")
	{
		ui::container container([&] {
			ui::container::info _;
			_.container = cargo;
			_.rm = rm;
			_.ui = ui;
			return _;
		}());
		REQUIRE(success(cargo.attach(
			{ 0, 0 },
			std::make_unique<module::cargo>(rm, random, YAML::Node()))));
		update_context context(rm, random);
		context.dt = .016;
		container.update(context);
		REQUIRE(container.children().size() == 1);
		WHEN("We click on the top left corner of the container")
		{
			ui::event event;
			// Setting to 32, because the container header is 32 pixels tall
			event.absolute_cursor = { 0, 32 };
			event.cursor = { 0, 32 };
			event.shift = false;
			let actions = container.click(event);
			THEN("It should return 'handled'") { REQUIRE(actions); }
			THEN("It should yield a single 'grab' action")
			{
				REQUIRE(actions->size() == 1);
				let action = actions->front().get();
				REQUIRE(
					dynamic_cast<ui::action::grab_from_container*>(action)
					!= nullptr);
			}
			THEN("The grab action should point to {0, 0} and the parent "
				 "container")
			{
				REQUIRE(actions->size() == 1);
				let action =
					dynamic_cast<const ui::action::grab_from_container&>(
						*actions->front());
				REQUIRE(&action.container() == &cargo);
				REQUIRE(action.pos().x == 0);
				REQUIRE(action.pos().y == 0);
			}
		}
		WHEN("We click on the top left corner of the container, when the "
			 "module is already grabbed")
		{
			ui.grabbed(*cargo.at({ 0, 0 }));
			ui::event event;
			// Setting to 32, because the container header is 32 pixels tall
			event.absolute_cursor = { 0, 32 };
			event.cursor = { 0, 32 };
			event.shift = false;
			let actions = container.click(event);
			THEN("It should return 'handled'") { REQUIRE(actions); }
			THEN("It should yield a single 'release_grabbed' action")
			{
				REQUIRE(actions->size() == 1);
				let action = actions->front().get();
				REQUIRE(
					dynamic_cast<ui::action::release_grabbed*>(action)
					!= nullptr);
			}
		}
		WHEN("We Shift-click on the top left corner should yield a 'recycle' "
			 "action")
		{
			ui::event event;
			// Setting to 32, because the container header is 32 pixels tall
			event.absolute_cursor = { 0, 32 };
			event.cursor = { 0, 32 };
			event.shift = true;
			let actions = container.click(event);
			THEN("It should return handled") { REQUIRE(actions); }
			THEN("It should yield a single 'recycle' action")
			{
				REQUIRE(actions->size() == 1);
				let action = actions->front().get();
				REQUIRE(dynamic_cast<ui::action::recycle*>(action) != nullptr);
			}
			THEN("That action should point to the 0,0 module and selected "
				 "container")
			{
				REQUIRE(actions->size() == 1);
				let action =
					dynamic_cast<const ui::action::recycle&>(*actions->front());
				REQUIRE(&action.container() == &cargo);
				REQUIRE(action.pos().x == 0);
				REQUIRE(action.pos().y == 0);
			}
		}
		WHEN("We click on the header")
		{
			ui::event event;
			// Setting to 32, because the container header is 32 pixels tall
			event.absolute_cursor = { 0, 64 };
			event.cursor = { 0, 64 };
			event.shift = false;
			let actions = container.click(event);
			THEN("It should return 'handled' to prevent event passthrough")
			{
				REQUIRE(actions);
			}
			THEN("It should return no actions") { REQUIRE(actions->empty()); }
		}
		WHEN("We click on the empty space below the module")
		{
			ui::event event;
			// Setting to 32, because the container header is 32 pixels tall
			event.absolute_cursor = { 0, 64 };
			event.cursor = { 0, 64 };
			event.shift = false;
			let actions = container.click(event);
			THEN("It should return 'handled' to prevent event passthrough")
			{
				REQUIRE(actions);
			}
			THEN("It should return no actions") { REQUIRE(actions->empty()); }
		}
		WHEN("We remove an item from the cargo module and call update")
		{
			(void)cargo.detach({ 0, 0 });
			container.update(context);
			THEN("The UI container should have no windows too")
			{
				REQUIRE(container.children().empty());
			}
		}
	}
}
} // namespace mark

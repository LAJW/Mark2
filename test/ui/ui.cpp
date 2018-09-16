#include <catch.hpp>
#include <ui/impl/ui.h>

SCENARIO("UI Implementation functions")
{
	using namespace mark::ui;
	GIVEN("ui::impl::drop_pos")
	{
		WHEN("supplied with 0, 0 and 1x1-size")
		{
			let drop_pos = impl::drop_pos({ 0, 0 }, { 1, 1 });
			THEN("should pick 0, 0")
			{
				REQUIRE(drop_pos.x == 0);
				REQUIRE(drop_pos.y == 0);
			}
		}
		WHEN("supplied with 0, 0 and 2x2-size")
		{
			let drop_pos = impl::drop_pos({ 0, 0 }, { 2, 2 });
			THEN("should pick -1, -1")
			{
				REQUIRE(drop_pos.x == -1);
				REQUIRE(drop_pos.y == -1);
			}
		}
		WHEN("supplied with 0, 0 and 3x2-size")
		{
			let drop_pos = impl::drop_pos({ 0, 0 }, { 3, 2 });
			THEN("should pick -1, -1")
			{
				REQUIRE(drop_pos.x == -1);
				REQUIRE(drop_pos.y == -1);
			}
		}
		WHEN("supplied with -1, 0 and 3x2-size")
		{
			let drop_pos = impl::drop_pos({ -1, 0 }, { 3, 2 });
			THEN("should pick -2, -1")
			{
				REQUIRE(drop_pos.x == -2);
				REQUIRE(drop_pos.y == -1);
			}
		}
		WHEN("supplied with -16, 0 and 3x2-size")
		{
			let drop_pos = impl::drop_pos({ -16, 0 }, { 3, 2 });
			THEN("should pick -2, -1")
			{
				REQUIRE(drop_pos.x == -2);
				REQUIRE(drop_pos.y == -1);
			}
		}
		WHEN("supplied with 0, -1 and 2x3-size")
		{
			let drop_pos = impl::drop_pos({ 0, -1 }, { 2, 3 });
			THEN("should pick -1, -2")
			{
				REQUIRE(drop_pos.x == -1);
				REQUIRE(drop_pos.y == -2);
			}
		}
	}
}
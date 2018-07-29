#include <catch.hpp>
#include <item/chaos_orb.h>
#include <item/shard.h>
#include <resource_manager.h>

SCENARIO("chaos_orb")
{
	using namespace mark;
	resource::manager_stub rm;
	GIVEN("an almost full stack of Chaos Orbs with couple slots to spare")
	{
		auto chaos_orb_ptr = std::make_unique<item::chaos_orb>(rm, [&] {
			auto node = YAML::Node();
			node["quantity"] = 15;
			return node;
		}());
		auto& chaos_orb = *chaos_orb_ptr;
		auto chaos_orb_item =
			std::unique_ptr<interface::item>(move(chaos_orb_ptr));
		WHEN("we try to stack it onto itself")
		{
			THEN("can_stack should return false")
			{
				REQUIRE(!chaos_orb.can_stack(chaos_orb));
			}
			THEN("stacking should have no effect")
			{
				chaos_orb.stack(chaos_orb_item);
				REQUIRE(chaos_orb_item != nullptr);
			}
		}
		WHEN("we stack a single Chaos Orb on top of it")
		{
			auto other = std::unique_ptr<interface::item>(
				std::make_unique<item::chaos_orb>(rm, [&] {
					auto node = YAML::Node();
					node["quantity"] = 1;
					return node;
				}()));
			REQUIRE(chaos_orb.can_stack(*other));
			chaos_orb.stack(other);
			THEN("original stack's quantity should be increased by one")
			{
				REQUIRE(chaos_orb.quantity() == 16);
			}
			THEN("new stack should be destroyed")
			{
				REQUIRE(other == nullptr);
			}
		}
		WHEN("we stack another almost full stack of Chaos Orbs on top of it")
		{
			auto other = std::unique_ptr<interface::item>(
				std::make_unique<item::chaos_orb>(rm, [&] {
					auto node = YAML::Node();
					node["quantity"] = 15;
					return node;
				}()));
			REQUIRE(chaos_orb.can_stack(*other));
			chaos_orb.stack(other);
			THEN("original stack should become full")
			{
				REQUIRE(chaos_orb.quantity() == 20);
			}
			THEN("new stack should NOT be destroyed")
			{
				REQUIRE(other != nullptr);
			}
			THEN("new stack's quantity should be reduced")
			{
				REQUIRE(other->quantity() == 10);
			}
		}
		WHEN("we try to stack a shard on top of it")
		{
			auto shard = std::unique_ptr<interface::item>(
				std::make_unique<item::shard>(rm, [&] {
					auto node = YAML::Node();
					node["quantity"] = 1;
					return node;
				}()));
			THEN("can_stack on the chaos_orb should return false")
			{
				REQUIRE(!chaos_orb.can_stack(*shard));
			}
			THEN("can_stack on the shard should return false")
			{
				REQUIRE(!chaos_orb.can_stack(*shard));
			}
			THEN("new stack should NOT be destroyed")
			{
				REQUIRE(shard != nullptr);
			}
			THEN("new stack's quantity should be reduced")
			{
				REQUIRE(shard->quantity() == 1);
			}
		}
	}
}

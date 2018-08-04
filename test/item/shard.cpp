#include <catch.hpp>
#include <item/shard.h>
#include <resource/manager.h>

SCENARIO("shard")
{
	using namespace mark;
	resource::manager_stub rm;
	GIVEN("an almost full stack of shards with couple slots to spare")
	{
		item::shard shard(rm, [&] {
			auto node = YAML::Node();
			node["quantity"] = 15;
			return node;
		}());
		WHEN("we stack a single shard on top of it")
		{
			auto other = std::unique_ptr<interface::item>(
				std::make_unique<item::shard>(rm, [&] {
					auto node = YAML::Node();
					node["quantity"] = 1;
					return node;
				}()));
			REQUIRE(shard.can_stack(*other));
			shard.stack(other);
			THEN("original stack's quantity should be increased by one")
			{
				REQUIRE(shard.quantity() == 16);
			}
			THEN("new stack should be destroyed")
			{
				REQUIRE(other == nullptr);
			}
		}
		WHEN("we stack another almost full stack of shards on top of it")
		{
			auto other = std::unique_ptr<interface::item>(
				std::make_unique<item::shard>(rm, [&] {
					auto node = YAML::Node();
					node["quantity"] = 15;
					return node;
				}()));
			REQUIRE(shard.can_stack(*other));
			shard.stack(other);
			THEN("original stack should become full")
			{
				REQUIRE(shard.quantity() == 20);
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
	}
}

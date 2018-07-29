#include <algorithm/range.h>
#include <catch.hpp>
#include <item/chaos_orb.h>
#include <item/shard.h>
#include <module/energy_generator.h>
#include <random.h>
#include <recycle.h>
#include <resource_manager.h>

SCENARIO("recycle function")
{
	mark::random_stub random;
	mark::resource::manager_stub rm;
	GIVEN("A vector containing a single energy generator")
	{
		auto energy_generator =
			std::make_unique<mark::module::energy_generator>(
				rm, random, YAML::Node());
		std::vector<mark::interface::item_ptr> items;
		items.emplace_back(move(energy_generator));
		WHEN("we recycle it")
		{
			let result = mark::recycle(rm, move(items));
			THEN("it should destroy all items") { REQUIRE(items.empty()); }
			THEN("it should return a vector with a single shard in it")
			{
				REQUIRE(result.size() == 1);
				REQUIRE(
					dynamic_cast<const mark::item::shard*>(result.front().get())
					!= nullptr);
			}
			THEN("Shard's quantity should be equal to 1")
			{
				REQUIRE(
					dynamic_cast<const mark::item::shard&>(*result.front())
						.quantity()
					== 1);
			}
			THEN("Shard's serialized quantity should be equal to 1")
			{
				YAML::Emitter out;
				result.front()->serialize(out);
				let result = YAML::Load(out.c_str());
				REQUIRE(result["quantity"].as<size_t>() == 1);
			}
		}
	}
	GIVEN("A vector containing 20 energy generators")
	{
		std::vector<mark::interface::item_ptr> items;
		for (let i : mark::range(20)) {
			(void)i;
			auto energy_generator =
				std::make_unique<mark::module::energy_generator>(
					rm, random, YAML::Node());
			items.emplace_back(move(energy_generator));
		}
		REQUIRE(items.size() == 20);
		WHEN("we recycle it")
		{
			let result = mark::recycle(rm, move(items));
			THEN("it should destroy all items") { REQUIRE(items.empty()); }
			THEN("it should return a vector with a single chaos orb in it")
			{
				REQUIRE(result.size() == 1);
				REQUIRE(
					dynamic_cast<const mark::item::chaos_orb*>(
						result.front().get())
					!= nullptr);
			}
			THEN("chaos orbs's quantity should be equal to 1")
			{
				REQUIRE(
					dynamic_cast<const mark::item::chaos_orb&>(*result.front())
						.quantity()
					== 1);
			}
			THEN("chaos orbs's serialized quantity should be equal to 1")
			{
				YAML::Emitter out;
				result.front()->serialize(out);
				let result = YAML::Load(out.c_str());
				REQUIRE(result["quantity"].as<size_t>() == 1);
			}
		}
	}
	GIVEN("A vector containing full stack of shards")
	{
		std::vector<mark::interface::item_ptr> items;
		auto energy_generator = std::make_unique<mark::item::shard>(rm, [&] {
			auto _ = YAML::Node();
			_["quantity"] = 20;
			return _;
		}());
		items.emplace_back(move(energy_generator));
		WHEN("we recycle it")
		{
			let result = mark::recycle(rm, move(items));
			THEN("it should destroy all items") { REQUIRE(items.empty()); }
			THEN("it should return a vector with a single chaos orb in it")
			{
				REQUIRE(result.size() == 1);
				REQUIRE(
					dynamic_cast<const mark::item::chaos_orb*>(
						result.front().get())
					!= nullptr);
			}
			THEN("chaos orbs's quantity should be equal to 1")
			{
				REQUIRE(
					dynamic_cast<const mark::item::chaos_orb&>(*result.front())
						.quantity()
					== 1);
			}
			THEN("chaos orbs's serialized quantity should be equal to 1")
			{
				YAML::Emitter out;
				result.front()->serialize(out);
				let result = YAML::Load(out.c_str());
				REQUIRE(result["quantity"].as<size_t>() == 1);
			}
		}
	}
	GIVEN("A vector containing almost full stack of shards and three items")
	{
		std::vector<mark::interface::item_ptr> items;
		auto energy_generator = std::make_unique<mark::item::shard>(rm, [&] {
			auto _ = YAML::Node();
			_["quantity"] = 19;
			return _;
		}());
		for (let i : mark::range(3)) {
			(void)i;
			auto shard = std::make_unique<mark::module::energy_generator>(
				rm, random, YAML::Node());
			items.emplace_back(move(shard));
		}
		items.emplace_back(move(energy_generator));
		WHEN("we recycle it")
		{
			let result = mark::recycle(rm, move(items));
			THEN("it should destroy all items") { REQUIRE(items.empty()); }
			THEN("item count should be 2") { REQUIRE(result.size() == 2); }
			THEN("first item should be a chaos orb")
			{
				REQUIRE(
					dynamic_cast<const mark::item::chaos_orb*>(
						result.front().get())
					!= nullptr);
			}
			THEN("there should be only one chaos orb on the stack")
			{
				REQUIRE(
					dynamic_cast<const mark::item::chaos_orb&>(*result.front())
						.quantity()
					== 1);
			}
			THEN("second item should be a stack of shards")
			{
				REQUIRE(
					dynamic_cast<const mark::item::shard*>(result.back().get())
					!= nullptr);
			}
			THEN("there should be two items on that stack")
			{
				REQUIRE(
					dynamic_cast<const mark::item::shard&>(*result.back())
						.quantity()
					== 2);
			}
		}
	}
	GIVEN("A vector containing 23 full stacks of shards")
	{
		std::vector<mark::interface::item_ptr> items;
		for (let i : mark::range(23)) {
			auto shard = std::make_unique<mark::item::shard>(rm, [&] {
				auto _ = YAML::Node();
				_["quantity"] = 20;
				return _;
			}());
			items.emplace_back(move(shard));
		}
		WHEN("we recycle it")
		{
			let result = mark::recycle(rm, move(items));
			THEN("it should destroy all items") { REQUIRE(items.empty()); }
			THEN("item count should be 2") { REQUIRE(result.size() == 2); }
			THEN("first item should be a full stack (20) of chaos orbs")
			{
				REQUIRE(
					dynamic_cast<const mark::item::chaos_orb*>(
						result.front().get())
					!= nullptr);
				REQUIRE(
					dynamic_cast<const mark::item::chaos_orb&>(*result.front())
						.quantity()
					== 20);
			}
			THEN("second should be a stack of three chaos orbs")
			{
				REQUIRE(
					dynamic_cast<const mark::item::chaos_orb*>(
						result.back().get())
					!= nullptr);
				REQUIRE(
					dynamic_cast<const mark::item::chaos_orb&>(*result.back())
						.quantity()
					== 3);
			}
		}
	}
}

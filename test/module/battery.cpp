#include <catch.hpp>
#include "../core_env.h"
#include <module/battery.h>

static auto attach_battery(
	mark::resource::manager& rm,
	mark::random& random,
	mark::unit::modular& modular,
	const YAML::Node& node) -> mark::module::battery&
{
	auto battery_ptr =
		std::make_unique<mark::module::battery>(rm, random, node);
	auto& battery = *battery_ptr;
	REQUIRE(!modular.attach(
		{ 1, -1 }, std::unique_ptr<mark::interface::item>(move(battery_ptr))));
	return battery;
}

template<typename T>
auto yaml_serialize(const T& entity)
{
	YAML::Emitter out;
	battery_env env;
	entity.serialize(out);
	std::istringstream in(out.c_str());
	return YAML::Load(in);
}

// Battery environment - A module environment with a battery attached to the
// module
struct battery_env : core_env
{
	mark::module::battery& battery;
	explicit battery_env(const YAML::Node& node = YAML::Node())
		: battery(attach_battery(rm, random, *modular, node))
	{}
};

TEST_CASE("Generic battery test")
{
	battery_env env;
	auto& battery = env.battery;

	// Generic module and item properties
	REQUIRE(battery.cur_health() == 100.f);
	REQUIRE(battery.max_health() == Approx(100.f));
	REQUIRE(battery.dead() == false);
	REQUIRE(battery.passive() == true);
	REQUIRE(battery.detachable() == true);
	let neighbors = battery.neighbors();
	REQUIRE(neighbors.size() == 1);
	REQUIRE(&neighbors[0].first.get() == env.modular->at({ -1, -1 }));
	REQUIRE(neighbors[0].second == 2);
	REQUIRE(env.battery.energy_ratio() == Approx(0.f));
}

TEST_CASE("Battery energy harvest test")
{
	YAML::Node config;
	config["cur_energy"] = 1000.f;
	battery_env env(config);
	auto& battery = env.battery;

	REQUIRE(env.battery.energy_ratio() == 1.f);
	let dt = .1;
	let my_energy = env.battery.harvest_energy(dt);
	REQUIRE(my_energy == Approx(.1f));
}

TEST_CASE("serialize")
{
	battery_env env;
	auto &battery = env.battery;
	let node = yaml_serialize(battery);
	REQUIRE(node["type"].as<std::string>() == mark::module::battery::type_name);
	REQUIRE(node["cur_health"].as<float>() == 100.f);
	REQUIRE(node["max_health"].as<float>() == 100.f);
	REQUIRE(node["cur_energy"].as<float>() == 0.f);
	REQUIRE(node["max_energy"].as<float>() == 1000.f);
}
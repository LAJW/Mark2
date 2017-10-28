#pragma once
#include "stdafx.h"
#include "interface_damageable.h"
#include "property.h"

namespace mark {

struct sprite;
class world;
class command;
struct tick_context;

namespace unit {
class base;
auto deserialise(
	world& world,
	const YAML::Node& node) ->
	std::shared_ptr<unit::base>;

// Part of the unit modifiable by world and nothing else
class base_ref {
public:
	friend world;
	auto world() noexcept -> mark::world&;
	auto world() const noexcept -> const mark::world&;
protected:
	base_ref(mark::world&);
	~base_ref() = default;
private:
	virtual void tick(tick_context& context) = 0;
	virtual void on_death(tick_context&) { /* no-op */ };
	std::reference_wrapper<mark::world> m_world;
};

class base:
	public base_ref,
	public std::enable_shared_from_this<unit::base> {
public:
	virtual ~base() = default;
	virtual void serialise(YAML::Emitter&) const;
	virtual void command(const command&) { };
	virtual auto dead() const -> bool = 0;
	virtual void activate(const std::shared_ptr<unit::base>&) { /* no-op */ };
	// Resolve references after deserializing
	virtual void resolve_ref(
		const YAML::Node&,
		const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>& units);

	Property<int> team = 0;
	Property<vector<double>> pos;
protected:
	base(mark::world& world, vector<double> pos);
	base(mark::world& world, const YAML::Node&);
};
}
}
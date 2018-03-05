#pragma once
#include "stdafx.h"
#include "interface_damageable.h"
#include "property.h"
#include "command.h"
#include "interface_world_object.h"

namespace mark {
namespace unit {

auto deserialise(
	world& world,
	const YAML::Node& node) ->
	std::shared_ptr<unit::base>;

// Part of the unit modifiable by world and nothing else
class base_ref : public interface::world_object {
public:
	friend mark::world;
	auto world() noexcept -> mark::world&;
	auto world() const noexcept -> const mark::world& override;
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
	virtual void command(const command::any&) { /* no-op */ };
	virtual auto dead() const -> bool = 0;
	// Resolve references after deserializing
	virtual void resolve_ref(
		const YAML::Node&,
		const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>& units);
	auto pos() const -> vector<double> override final;
	void pos(const vector<double> &);
	auto team() const -> size_t final override;
	void team(size_t);

	struct info {
		mark::world* world = nullptr;
		vector<double> pos;
		int team = 0;
	};
protected:
	base(const info&);
	base(mark::world& world, const YAML::Node&);
private:
	vector<double> m_pos;
	size_t m_team;
};
}
}
#pragma once
#include "command.h"
#include "interface/damageable.h"
#include "interface/world_object.h"
#include "property.h"
#include "stdafx.h"

namespace mark {
namespace unit {

auto deserialize(world& world, const YAML::Node& node)
	-> std::shared_ptr<unit::base>;

// Part of the unit modifiable by world and nothing else
class base_ref : public interface::world_object
{
public:
	friend mark::world;
	auto world() noexcept -> mark::world&;
	auto world() const noexcept -> const mark::world& override;

protected:
	base_ref(mark::world&);
	~base_ref() = default;

private:
	virtual void update(update_context& context) = 0;
	virtual void on_death(update_context&){ /* no-op */ };
	ref<mark::world> m_world;
};

class base
	: public base_ref
	, public std::enable_shared_from_this<unit::base>
{
public:
	virtual ~base() = default;
	virtual void serialize(YAML::Emitter&) const;
	virtual void command(const command::any&){ /* no-op */ };
	virtual auto dead() const -> bool = 0;
	// Resolve references after deserializing
	virtual void resolve_ref(
		const YAML::Node&,
		const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>& units);
	auto pos() const -> vd override final;
	void pos(const vd&);
	auto team() const -> size_t final override;
	void team(size_t);

	struct info
	{
		mark::world* world = nullptr;
		vd pos;
		size_t team = 0;
	};

protected:
	base(const info&);
	base(mark::world& world, const YAML::Node&);

private:
	vd m_pos;
	size_t m_team;
};
} // namespace unit
} // namespace mark

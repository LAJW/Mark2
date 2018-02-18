#pragma once
#include "stdafx.h"
#include "module_base.h"
#include "targeting_system.h"

namespace mark {
namespace module {

class base_turret : public module::base {
public:
	auto passive() const noexcept -> bool override;
	void command(const command::any&) override;
protected:
	base_turret(
		vector<unsigned> size,
		const std::shared_ptr<const resource::image>& image);
	base_turret(resource::manager&, const YAML::Node& node);

	void tick();
	// Can the inheriting turret perform a shot in this frame. Returns true
	// if yes, false otherwise. Mutable, as requesting charge might reset the
	// internal clock.
	auto request_charge() -> bool;
	// Target at which turret is pointing
	auto target() const -> std::optional<vector<double>>;
	void serialise(YAML::Emitter&) const;
	// Level of turret's charge (between 0 and 1)
private:
	targeting_system m_targeting_system;
};
} }
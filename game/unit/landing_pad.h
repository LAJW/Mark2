﻿#pragma once
#include "interface/has_bindings.h"
#include "stdafx.h"
#include "unit/activable.h"

namespace mark {
namespace unit {
class landing_pad final
	: public unit::activable
	, public interface::has_bindings
{
public:
	static constexpr const char* type_name = "unit_landing_pad";
	static constexpr let radius = 300.0;

	landing_pad(mark::world& world, const YAML::Node&);
	landing_pad(const info&);
	auto dead() const -> bool override { return false; };
	[[nodiscard]] auto use(const std::shared_ptr<unit::modular>& by)
		-> std::error_code override;
	void command(const mark::command::any& command) override;
	void serialise(YAML::Emitter&) const override;
	void resolve_ref(
		const YAML::Node&,
		const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>& units)
		override;
	auto bindings() const -> interface::has_bindings::bindings_t override;
	auto ship() -> std::shared_ptr<unit::modular>;
	auto ship() const -> std::shared_ptr<const unit::modular>;

private:
	void tick(tick_context& context) override;

	std::shared_ptr<const resource::image> m_image;
	std::weak_ptr<unit::modular> m_ship;
};
} // namespace unit
} // namespace mark
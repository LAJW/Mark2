﻿#pragma once
#include "base.h"

namespace mark {
namespace unit {
class bucket final : public unit::base {
public:
	static constexpr const char* type_name = "unit_bucket";

	bucket(mark::world& world, const YAML::Node&);
	struct info : unit::base::info {
		std::unique_ptr<module::base> module;
	};
	bucket(info);
	auto dead() const -> bool override;
	auto release() -> std::unique_ptr<module::base>;
	void insert(std::unique_ptr<module::base>);
	void serialise(YAML::Emitter& out) const override;

private:
	void tick(tick_context& context) override;
	std::unique_ptr<module::base> m_module;
	float m_direction = NAN;
};
} // namespace unit
} // namespace mark
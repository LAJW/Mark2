#pragma once
#include "base.h"

namespace mark {
namespace unit {
class bucket final : public unit::base
{
public:
	static constexpr const char* type_name = "unit_bucket";

	bucket(mark::world& world, const YAML::Node&);
	struct info : unit::base::info
	{
		interface::item_ptr item;
		float rotation = 0.f;
	};
	bucket(info);
	auto dead() const -> bool override;
	auto release() -> interface::item_ptr;
	void insert(interface::item_ptr);
	void serialize(YAML::Emitter& out) const override;
	auto radius() const -> double override;

private:
	void update(update_context& context) override;
	interface::item_ptr m_item;
	float m_direction = NAN;
	float m_rotation = 0;
};
} // namespace unit
} // namespace mark
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
		std::unique_ptr<interface::item> item;
		float rotation = 0.f;
	};
	bucket(info);
	auto dead() const -> bool override;
	auto release() -> std::unique_ptr<interface::item>;
	void insert(std::unique_ptr<interface::item>);
	void serialise(YAML::Emitter& out) const override;

private:
	void update(update_context& context) override;
	std::unique_ptr<interface::item> m_item;
	float m_direction = NAN;
	float m_rotation = 0;
};
} // namespace unit
} // namespace mark
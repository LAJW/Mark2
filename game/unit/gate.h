#pragma once
#include "activable.h"
#include <model_animated.h>
#include <stdafx.h>

namespace mark {
namespace unit {
class gate final : public activable
{
public:
	static constexpr const char* type_name = "gate";
	static constexpr let size = 256.;

	struct info : base::info
	{
		bool inverted;
	};
	gate(const info&);
	gate(mark::world& world, const YAML::Node&);
	auto dead() const -> bool override { return false; };
	[[nodiscard]] auto use(const shared_ptr<unit::modular>& by)
		-> std::error_code override;
	void serialize(YAML::Emitter&) const override;
	auto radius() const -> double override { return size / 2.; }

private:
	void update(update_context& context) override;
	resource::image_ptr m_im_base;
	resource::image_ptr m_im_enclosure;
	mark::model::animated m_rotor;
	// If set to true, gate leads to the previous world
	bool m_inverted;
};
} // namespace unit
} // namespace mark

#pragma once
#include "stdafx.h"
#include "unit_activable.h"
#include "model_animated.h"

namespace mark {
namespace unit {
class gate final : public activable {
public:
	static constexpr const char* type_name = "gate";

	struct info : base::info {
		bool inverted;
	};
	gate(const info&);
	gate(mark::world& world, const YAML::Node&);
	auto dead() const -> bool override { return false; };
	[[nodiscard]] auto use(
		const std::shared_ptr<unit::modular>& by)
		-> std::error_code override;
	void serialise(YAML::Emitter&) const override;
private:
	void tick(tick_context& context) override;
	std::shared_ptr<const resource::image> m_im_base;
	std::shared_ptr<const resource::image> m_im_enclosure;
	mark::model::animated m_rotor;
	// If set to true, gate leads to the previous world
	bool m_inverted;
};
}
}

#pragma once
#include "base.h"
#include <lfo.h>
#include <model_shield.h>
#include <stdafx.h>

namespace mark {
namespace module {
class shield_generator final : public base
{
public:
	static constexpr const char* type_name = "shield_generator";

	shield_generator(resource::manager&, const YAML::Node&);
	auto damage(const interface::damageable::info&) -> bool override;
	auto describe() const -> std::string override;
	auto collide(const segment_t&) -> std::optional<std::pair<
		std::reference_wrapper<interface::damageable>,
		vector<double>>> override;
	auto shield() const noexcept -> float;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
	void command(const command::any& any) override;

private:
	void update(update_context& context) override;
	void render(update_context& context) const;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	void bind(mark::property_manager&) override;

	static constexpr let default_radius = 128.f;
	static constexpr let default_shield_per_energy = 10.f;

	const std::shared_ptr<const resource::image> m_im_generator;
	float m_max_shield = 1000.f;
	float m_radius = default_radius;
	float m_shield_per_energy = 10.f;
	model::shield m_model_shield;
	float m_cur_shield = 1000.f;
	bool m_on = true;
};
} // namespace module
} // namespace mark

#pragma once
#include "base.h"
#include <stdafx.h>

namespace mark {
namespace module {
class engine final : public module::base
{
public:
	static constexpr const char* type_name = "module_engine";

	engine(resource::manager&, const YAML::Node&);
	auto describe() const -> std::string override;
	auto global_modifiers() const -> module::modifiers override;
	void command(const command::any&) override;
	void serialise(YAML::Emitter&) const override;
	auto reserved() const noexcept -> reserved_kind override;
	auto passive() const noexcept -> bool override;

private:
	void tick(tick_context& context) override;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	void bind(mark::property_manager&) override;

	std::shared_ptr<const resource::image> m_image_base;
	enum class state
	{
		off,
		manual,
		toggled
	};
	state m_state = state::off;
};
} // namespace module
} // namespace mark
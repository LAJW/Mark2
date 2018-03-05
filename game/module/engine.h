#pragma once
#include <stdafx.h>
#include "base.h"

namespace mark {
namespace module {
class engine final : public module::base {
public:
	static constexpr const char* type_name = "module_engine";

	engine(resource::manager&, const YAML::Node&);
	engine(resource::manager& manager);
	auto describe() const->std::string override;
	auto global_modifiers() const->module::modifiers override;
	void command(const command::any&) override;
	void serialise(YAML::Emitter&) const override;
	auto reserved() const noexcept -> reserved_type override;
	auto passive() const noexcept -> bool override;
private:
	void tick(tick_context& context) override;

	std::shared_ptr<const resource::image> m_image_base;
	enum class state { off, manual, toggled };
	state m_state = state::off;
};
}
}
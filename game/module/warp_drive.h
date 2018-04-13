#pragma once
#include "base.h"
#include <stdafx.h>

namespace mark {
namespace module {
class warp_drive final : public module::base
{
public:
	static constexpr const char* type_name = "module_warp_drive";

	warp_drive(resource::manager&, const YAML::Node&);
	auto describe() const -> std::string override;
	void command(const command::any&) override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

private:
	void update(update_context& context) override;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	void bind(mark::property_manager&) override;

	std::shared_ptr<const resource::image> m_image_base;
};
} // namespace module
} // namespace mark

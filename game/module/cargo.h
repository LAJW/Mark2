#pragma once
#include "base.h"
#include <exception.h>
#include <interface/container.h>
#include <lfo.h>
#include <resource_image.h>
#include <stdafx.h>

namespace mark {
namespace module {
class cargo final : public module::base, public interface::container {
public:
	static constexpr const char* type_name = "module_cargo";

	cargo(resource::manager&, const YAML::Node&);

	[[nodiscard]] auto
	attach(const vector<int>& pos, std::unique_ptr<module::base>& module)
		-> std::error_code override;
	auto can_attach(const vector<int>& pos, const module::base& module) const
		-> bool override;
	auto at(const vector<int>& pos) -> module::base* override;
	auto at(const vector<int>& pos) const -> const module::base* override;
	auto detach(const vector<int>& pos)
		-> std::unique_ptr<module::base> override;
	auto detachable() const -> bool override;
	auto describe() const -> std::string override;
	void on_death(tick_context& context) override;
	void serialise(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

	// try to push element to the container
	[[nodiscard]] std::error_code push(std::unique_ptr<module::base>& module);
	auto interior_size() const -> vector<int>; // size of the contents of the
											   // cargo hold in modular units
	auto modules() -> std::vector<std::unique_ptr<module::base>>&;
	auto modules() const -> const std::vector<std::unique_ptr<module::base>>&;

private:
	void tick(tick_context& context) override;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	void bind(mark::property_manager&) override;

	std::shared_ptr<const resource::image> m_grid_bg;
	std::shared_ptr<const resource::image> m_im_body;
	std::shared_ptr<const resource::image> m_im_light;
	lfo m_lfo;
	std::vector<std::unique_ptr<module::base>> m_modules;
};
} // namespace module
} // namespace mark
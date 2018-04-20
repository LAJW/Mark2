#pragma once
#include "base.h"
#include <exception.h>
#include <interface/container.h>
#include <lfo.h>
#include <resource_image.h>
#include <stdafx.h>

namespace mark {
namespace module {
class cargo final
	: public module::base
	, public interface::container
{
public:
	static constexpr const char* type_name = "module_cargo";

	cargo(resource::manager&, const YAML::Node&);

	[[nodiscard]] auto
	attach(const vector<int>& pos, std::unique_ptr<interface::item>& item)
		-> std::error_code override;
	auto can_attach(const vector<int>& pos, const interface::item& item) const
		-> bool override;
	auto at(const vector<int>& pos) -> interface::item* override;
	auto at(const vector<int>& pos) const -> const interface::item* override;
	auto detach(const vector<int>& pos)
		-> std::unique_ptr<interface::item> override;
	auto detachable() const -> bool override;
	auto describe() const -> std::string override;
	void on_death(update_context& context) override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

	// try to push element to the container
	[[nodiscard]] std::error_code push(std::unique_ptr<interface::item>& item);
	auto interior_size() const -> vector<int>; // size of the contents of the
											   // cargo hold in modular units
	auto items() -> std::vector<std::unique_ptr<interface::item>>&;
	auto items() const -> const std::vector<std::unique_ptr<interface::item>>&;

private:
	void update(update_context& context) override;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);

	std::shared_ptr<const resource::image> m_grid_bg;
	std::shared_ptr<const resource::image> m_im_body;
	std::shared_ptr<const resource::image> m_im_light;
	lfo m_lfo;
	std::vector<std::unique_ptr<interface::item>> m_items;
};
} // namespace module
} // namespace mark
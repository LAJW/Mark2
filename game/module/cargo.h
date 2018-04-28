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

	[[nodiscard]] auto attach(const vi32& pos, interface::item_ptr& item)
		-> std::error_code override;
	auto can_attach(const vi32& pos, const interface::item& item) const
		-> bool override;
	auto at(const vi32& pos) -> interface::item* override;
	auto at(const vi32& pos) const -> const interface::item* override;
	auto detach(const vi32& pos) -> interface::item_ptr override;
	auto detachable() const -> bool override;
	auto describe() const -> std::string override;
	void on_death(update_context& context) override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

	// try to push element to the container
	[[nodiscard]] std::error_code push(interface::item_ptr& item);
	auto interior_size() const -> vi32; // size of the contents of the
										// cargo hold in modular units
	auto items() -> std::vector<interface::item_ptr>&;
	auto items() const -> const std::vector<interface::item_ptr>&;

private:
	void update(update_context& context) override;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);

	resource::image_ptr m_grid_bg;
	resource::image_ptr m_im_body;
	resource::image_ptr m_im_light;
	lfo m_lfo;
	std::vector<interface::item_ptr> m_items;
};
} // namespace module
} // namespace mark

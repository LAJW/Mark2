#pragma once
#include "base.h"
#include <exception.h>
#include <interface/container.h>
#include <lfo.h>
#include <resource/image.h>
#include <stdafx.h>

namespace mark {
namespace module {
class cargo final
	: public module::base
	, public interface::container
{
public:
	static constexpr const char* type_name = "module_cargo";

	cargo(resource::manager&, mark::random& random, const YAML::Node&);

	[[nodiscard]] auto attach(vi32 pos, interface::item_ptr&& item)
		-> std::error_code override;
	auto can_attach(vi32 pos, const interface::item& item) const
		-> bool override;
	auto at(vi32 pos) -> optional<interface::item&> override;
	auto at(vi32 pos) const -> optional<const interface::item&> override;
	auto pos_at(vi32 pos) const noexcept -> std::optional<vi32> override;
	auto detach(vi32 pos) -> interface::item_ptr override;
	auto can_detach(vi32 pos) const noexcept -> bool override;
	auto detachable() const -> bool override;
	auto describe() const -> std::string override;
	void on_death(update_context& context) override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

	// Size of the grid of the cargo hold in module units
	auto interior_size() const -> vi32;

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

// Store the item in the container. If passed item is stackable, it might be
// consumed by another stack. If there's no space for the item, function
// returns "occupied" error
[[nodiscard]] std::error_code
push(module::cargo& cargo, interface::item_ptr&& item);

// Try to stack an item. Unlike push, it only stores the item if another
// stackable item of its kind already exists in the container. Destroys the
// item if it's fully stacked
void stack(module::cargo& cargo, interface::item_ptr&& item);

} // namespace module
} // namespace mark

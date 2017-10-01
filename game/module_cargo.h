#pragma once
#include "stdafx.h"
#include "exception.h"
#include "module_base.h"
#include "resource_image.h"
#include "lfo.h"

namespace mark {
struct tick_context;
namespace resource {
class manager;
}
namespace module {
class cargo : public mark::module::base {
public:
	static constexpr const char* type_name = "module_cargo";

	cargo(mark::resource::manager&, const YAML::Node&);
	cargo(mark::resource::manager& manager);
	virtual void tick(mark::tick_context& context) override;
	auto modules() -> std::vector<std::unique_ptr<mark::module::base>>&;
	auto modules() const -> const std::vector<std::unique_ptr<mark::module::base>>&;
	[[nodiscard]] error::code drop(mark::vector<int> pos, std::unique_ptr<mark::module::base>& module);
	auto module(mark::vector<int> pos) -> mark::module::base*;
	auto module(mark::vector<int> pos) const->const mark::module::base*;
	auto pick(mark::vector<int> pos) -> std::unique_ptr<mark::module::base>;
	auto interior_size() const -> mark::vector<int>; // size of the contents of the cargo hold in modular units
	auto detachable() const -> bool override;
	auto describe() const -> std::string;
	void on_death(mark::tick_context& context);
	// try to push element to the container
	[[nodiscard]] mark::error::code push(std::unique_ptr<mark::module::base>& module);
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
private:
	std::shared_ptr<const mark::resource::image> m_grid_bg;
	std::shared_ptr<const mark::resource::image> m_im_body;
	std::shared_ptr<const mark::resource::image> m_im_light;
	mark::lfo m_lfo;
	std::vector<std::unique_ptr<mark::module::base>> m_modules;
};
}
}
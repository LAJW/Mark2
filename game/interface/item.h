#pragma once
#include <exception.h>
#include <stdafx.h>

namespace mark {
namespace interface {

class item
{
public:
	virtual ~item() = default;
	virtual auto size() const -> vu32 = 0;
	virtual auto thumbnail() const -> resource::image_ptr = 0;
	struct use_on_result
	{
		std::error_code error;
		bool consumed = false;
	};
	virtual auto use_on(
		mark::random& random,
		const std::unordered_map<std::string, YAML::Node>& blueprints,
		module::base& item) -> use_on_result
	{
		(void)random;
		(void)blueprints;
		(void)item;
		use_on_result result;
		result.error = error::code::bad_pos;
		result.consumed = false;
		return result;
	}
	virtual void serialize(YAML::Emitter& out) const = 0;
	virtual auto describe() const -> std::string = 0;
	// Stack modules on top of each other, return error if cannot stack
	// Remove module from passed in unique_ptr on success
	virtual void stack(interface::item_ptr& item) = 0;
	virtual bool can_stack(const interface::item& item) const = 0;
	virtual auto quantity() const -> size_t = 0;
};
} // namespace interface
} // namespace mark

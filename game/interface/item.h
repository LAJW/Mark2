#pragma once
#include <exception.h>
#include <stdafx.h>

namespace mark {
namespace interface {

class item {
public:
	virtual ~item() = default;
	virtual auto size() const -> vector<unsigned> = 0;
	virtual auto thumbnail() const
		-> std::shared_ptr<const resource::image> = 0;
	struct use_on_result {
		std::error_code error;
		bool consumed = false;
	};
	virtual auto use_on(
		resource::manager& rm,
		const std::unordered_map<std::string, YAML::Node>& blueprints,
		module::base& item) -> use_on_result
	{
		(void)rm;
		(void)blueprints;
		(void)item;
		use_on_result result;
		result.error = error::code::bad_pos;
		result.consumed = false;
		return result;
	}
	virtual void serialise(YAML::Emitter& out) const = 0;
	virtual auto describe() const -> std::string = 0;
};
} // namespace interface
} // namespace mark

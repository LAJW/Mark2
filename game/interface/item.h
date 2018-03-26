#pragma once
#include <exception.h>
#include <stdafx.h>

namespace mark {
namespace interface {

class item {
public:
	virtual ~item() = default;
	virtual auto size() const -> vector<unsigned> = 0;
	virtual auto thumbnail() const -> std::shared_ptr<const resource::image> = 0;
	virtual auto use_on(
		resource::manager& rm,
		const std::unordered_map<std::string, YAML::Node>& blueprints,
		module::base& item) -> std::error_code
	{
		(void)rm;
		(void)blueprints;
		(void)item;
		return error::code::bad_pos;
	}
	virtual void serialise(YAML::Emitter& out) const = 0;
	virtual auto describe() const -> std::string = 0;
};
} // namespace interface
} // namespace mark

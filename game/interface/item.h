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
	virtual auto use_on(module::base&) -> std::error_code
	{
		return error::code::bad_pos;
	}
	virtual void serialise(YAML::Emitter& out) const = 0;
	virtual auto describe() const -> std::string = 0;
};
} // namespace interface
} // namespace mark

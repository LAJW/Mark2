#pragma once
#include "stdafx.h"

namespace mark { namespace interface {

class serializable {
public:
	virtual void serialize(YAML::Emitter& out) const = 0;
	inline auto id() const -> uint64_t
	{
		return reinterpret_cast<uint64_t>(this);
	}
protected:
	~serializable() = default;
};

} }
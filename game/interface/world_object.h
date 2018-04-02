#pragma once
#include <stdafx.h>

namespace mark {
class world;

namespace interface {
class world_object
{
public:
	virtual auto pos() const -> vector<double> = 0;
	virtual auto world() const -> const world& = 0;
	virtual auto team() const -> size_t = 0;

protected:
	~world_object() = default;
};
} // namespace interface
} // namespace mark

#pragma once
#include "base.h"
#include <interface/damageable.h>

namespace mark {
namespace unit {
class damageable
	: public unit::base
	, public interface::damageable
{
public:
	virtual auto collide(const segment_t&)
		-> std::optional<std::pair<interface::damageable&, vd>> = 0;
	virtual auto collide(vd center, double radius)
		-> std::vector<ref<interface::damageable>> = 0;

protected:
	template <typename... Args>
	inline damageable(Args&&... args)
		: base(std::forward<Args>(args)...)
	{}
};
} // namespace unit
} // namespace mark

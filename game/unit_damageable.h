#pragma once
#include "stdafx.h"
#include "unit_base.h"
#include "idamageable.h"

namespace mark {
namespace unit {

class damageable : public mark::unit::base, public mark::idamageable {
public:
	virtual auto collide(const mark::segment_t&) ->
		std::pair<mark::idamageable*, mark::vector<double>> = 0;
	virtual auto collide(mark::vector<double> center, float radius) ->
		std::vector<std::reference_wrapper<mark::idamageable>> = 0;
protected:
	template<typename...Args>
	inline damageable(Args&&... args)
		: base(std::forward<Args>(args)...) {}
};

} }
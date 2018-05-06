#pragma once
#include <stdafx.h>

namespace mark {
namespace interface {

class damageable
{
public:
	struct info
	{
		// List of pointers to a set used to store damaged objects
		// Do not dereference pointers stored here, they're only there
		// to ignore damaged objects. Cannot be null upon invoking
		// damage(). idamageable upon taking damage should store itself
		// here
		std::unordered_set<not_null<damageable*>>* damaged = nullptr;
		std::unordered_set<not_null<damageable*>>* knocked = nullptr;
		vd pos;
		float physical = 0.f;
		float energy = 0.f;
		float critical_chance = 0.1f;
		float critical_multiplier = 1.50f;
		float stun_chance = 0.0f;
		float stun_duration = 0.f;
		double knockback = 0.f;
		size_t team = 0;
		bool pierce = false;
	};
	// try to damage object
	// Stores damaged object pointers in info.damaged
	// returns true if something has been damaged
	virtual bool damage(const damageable::info&) = 0;
	virtual bool reflective() const { return false; }

protected:
	~damageable() = default;
};
} // namespace interface
} // namespace mark

#pragma once
#include <unordered_set>
#include "vector.h"

namespace mark {
	class idamageable {
	public:
		struct attributes {
			// pointer to a set used to store damaged objects
			// do not dereference pointers stored here, they're only there to ignore damaged objects
			// cannot be null upon invoking damage()
			// idamageable upon taking damage should store itself here
			std::unordered_set<idamageable*>* damaged = nullptr;
			mark::vector<double> pos;
			float physical = 0.f;
			float energy = 0.f;
			size_t team = 0;
			bool pierce = false;
		};
		// try to damage object, return damaged object on success
		// if no object has been damaged, return empty vector
		// returns true if something has been damaged
		virtual bool damage(const idamageable::attributes& attr) { return false; };
	};
}
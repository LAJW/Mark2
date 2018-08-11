#pragma once
#include <stdafx.h>

/// \file Internal helper functions for the healing turret

namespace mark {

[[nodiscard]] bool
in_range(const module::base& a, const module::base& b, double range);

[[nodiscard]] optional<const module::base&>
neighbor_at_pos_in_range(const module::base& root, vi32 pos, double range);

[[nodiscard]] std::unordered_set<not_null<const module::base*>>
neighbors_in_radius(const module::base& root, double radius);

[[nodiscard]] const mark::module::base*
most_damaged_neighbor_in_range(const module::base& root, double range);

} // namespace mark

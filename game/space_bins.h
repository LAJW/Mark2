#pragma once

#include <algorithm.h>

namespace mark {

namespace unit {
class base;
}

template <typename T>
class space_bins final
{
private:
	using element_type = not_null<shared_ptr<T>>;

public:
	space_bins(vector<std::size_t> size, vd world_min, vd world_max)
		: m_size{ size }
		, m_voxels{ size.x * size.y, std::vector<element_type>() }
		, m_world_min{ world_min }
		, m_world_max{ world_max }
	{}

	auto size() const { return m_size; }
	auto world_min() const { return m_world_min; }
	auto world_max() const { return m_world_max; }

	auto at(vector<std::size_t> p) -> std::vector<element_type>&
	{
		return at_impl(*this, p);
	}

	auto at(vector<std::size_t> p) const -> const std::vector<element_type>&
	{
		return at_impl(*this, p);
	}

private:
	template <typename U>
	static auto at_impl(U& self, vector<std::size_t> p) -> decltype(self.at(p))
	{
		return self.m_voxels.at(p.y * self.size().x + p.x);
	}

	vector<std::size_t> m_size;
	std::vector<std::vector<element_type>> m_voxels;
	vd m_world_min;
	vd m_world_max;
};

template <typename T>
auto compute_index(const space_bins<T>& bins, vd pos)
{
	let world_size = bins.world_max() - bins.world_min();
	let world_pos = pos - bins.world_min();
	let world_dim = bins.size();
	return vector<std::size_t>{ max(
		vd{ 0.0, 0.0 },
		vd{ (world_pos.x / world_size.x) * world_dim.x,
			(world_pos.y / world_size.y) * world_dim.y }) };
}

template <typename U>
auto range_for(const space_bins<U>& bins, vd pos, double radius)
{
	let top_left = compute_index(bins, pos - vd{ radius, radius });
	let unbound_bottom_right = vector<std::size_t>{ 1, 1 }
		+ compute_index(bins, pos + vd{ radius, radius });
	let bottom_right = min(bins.size(), unbound_bottom_right);
	return mark::range(top_left, bottom_right);
}

template <typename It, typename T>
void divide_space(It first_unit, It last_unit, space_bins<T>& bins)
{
	let world_dim = bins.size();
	for (let ind : range(world_dim)) {
		bins.at(ind).clear();
	}

	std::for_each(first_unit, last_unit, [&bins](let& unit) {
		for (let ind : range_for(bins, unit->pos(), unit->radius())) {
			bins.at(ind).emplace_back(unit);
		}
	});
}

template <typename unit_type = unit::base, typename T>
auto check_proximity(
	not_null<shared_ptr<unit::base>> unit,
	vd pos,
	double radius,
	T pred) -> shared_ptr<unit_type>
{
	if (length(unit->pos() - pos) >= radius + unit->radius()) {
		return nullptr;
	}
	let derived = std::dynamic_pointer_cast<unit_type>(unit.get());
	return (derived && pred(*derived)) ? derived : nullptr;
}

template <typename unit_type = unit::base, typename T, typename U>
auto find(const space_bins<U>& bins, vd pos, double radius, T pred)
{
	std::unordered_set<not_null<shared_ptr<unit_type>>> ret;
	for (let ind : range_for(bins, pos, radius)) {
		for (let& unit : bins.at(ind)) {
			if (let ptr = check_proximity<unit_type>(unit, pos, radius, pred)) {
				ret.insert(ptr);
			}
		}
	}
	return std::vector<decltype(ret)::value_type>{ ret.begin(), ret.end() };
}

template <typename unit_type = unit::base, typename T, typename U>
auto find_one(const space_bins<U>& bins, vd pos, double radius, T pred)
	-> shared_ptr<unit_type>
{
	for (let ind : range_for(bins, pos, radius)) {
		for (let& unit : bins.at(ind)) {
			if (let ptr = check_proximity<unit_type>(unit, pos, radius, pred)) {
				return ptr;
			}
		}
	}
	return {};
}

} // namespace mark

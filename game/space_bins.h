#pragma once

#include <algorithm.h>

namespace mark {

namespace unit {
class base;
}

template <typename T>
class space_bins final
{
public:
	space_bins(
		vector<std::size_t> size,
		vector<double> world_min,
		vector<double> world_max)
		: m_size{ size }
		, m_voxels{ size.x * size.y, std::vector<std::shared_ptr<T>>() }
		, m_world_min{ world_min }
		, m_world_max{ world_max }
	{}

	auto size() const { return m_size; }
	auto world_min() const { return m_world_min; }
	auto world_max() const { return m_world_max; }

	auto at(vector<std::size_t> p) -> std::vector<std::shared_ptr<T>>&
	{
		return at_impl(*this, p);
	}

	auto at(vector<std::size_t> p) const
		-> const std::vector<std::shared_ptr<T>>&
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
	std::vector<std::vector<std::shared_ptr<T>>> m_voxels;
	vector<double> m_world_min;
	vector<double> m_world_max;
};

template <typename T>
auto compute_index(const space_bins<T>& bins, vector<double> pos)
{
	let world_size = bins.world_max() - bins.world_min();
	let world_pos = pos - bins.world_min();
	let world_dim = bins.size();
	return vector<std::size_t>{ max(
		vector<double>{ 0.0, 0.0 },
		vector<double>{ (world_pos.x / world_size.x) * world_dim.x,
						(world_pos.y / world_size.y) * world_dim.y }) };
}

template <typename It, typename T>
void divide_space(It first_unit, It last_unit, space_bins<T>& bins)
{
	let world_dim = bins.size();
	for (let ind : range(world_dim)) {
		bins.at(ind).clear();
	}

	std::for_each(first_unit, last_unit, [&bins](let& unit) {
		bins.at(compute_index(bins, unit->pos())).emplace_back(unit);
	});
}

template <typename unit_type = unit::base, typename T>
auto check_proximity(
	std::shared_ptr<unit::base> unit,
	vector<double> pos,
	double radius,
	T pred) -> std::shared_ptr<unit_type>
{
	if (length(unit->pos() - pos) < radius && pred(*unit)) {
		if (let& derived = std::dynamic_pointer_cast<unit_type>(unit)) {
			return derived;
		}
	}
	return nullptr;
}

template <typename unit_type = unit::base, typename T, typename U>
auto find(const space_bins<U>& bins, vector<double> pos, double radius, T pred)
{
	std::vector<std::shared_ptr<unit_type>> ret;
	for (let ind : range(
			 compute_index(bins, pos - vector<double>{ radius, radius }),
			 min(bins.size(),
				 vector<std::size_t>{ 1, 1 }
					 + compute_index(
						   bins, pos + vector<double>{ radius, radius })))) {
		for (let& unit : bins.at(ind)) {
			if (let ptr = check_proximity<unit_type>(unit, pos, radius, pred)) {
				ret.emplace_back(ptr);
			}
		}
	}
	return ret;
}

template <typename unit_type = unit::base, typename T, typename U>
auto find_one(
	const space_bins<U>& bins,
	vector<double> pos,
	double radius,
	T pred) -> std::shared_ptr<unit_type>
{
	for (let ind : range(
			 compute_index(bins, pos - vector<double>{ radius, radius }),
			 min(bins.size(),
				 vector<std::size_t>{ 1, 1 }
					 + compute_index(
						   bins, pos + vector<double>{ radius, radius })))) {
		for (let& unit : bins.at(ind)) {
			if (let ptr = check_proximity<unit_type>(unit, pos, radius, pred)) {
				return ptr;
			}
		}
	}
	return {};
}

} // namespace mark

#pragma once

#include <unit/base.h>
#include <algorithm.h>

namespace mark {

class space_bins final {
public:
	space_bins() = default;

	space_bins(
		vector<std::size_t> dimensions_in_voxels,
		vector<double> world_min,
		vector<double> world_max)
		: m_voxels{dimensions_in_voxels.x,
				   std::vector<std::vector<std::shared_ptr<unit::base>>>{
					   dimensions_in_voxels.y}}
		, m_world_min{world_min}
		, m_world_max{world_max}
	{
	}

	auto dimensions_in_voxels() const -> vector<std::size_t>
	{
		return {m_voxels.size(), m_voxels.at(0).size()};
	}

	auto world_min() const { return m_world_min; }
	auto world_max() const { return m_world_max; }

	auto at(vector<std::size_t> p) -> std::vector<std::shared_ptr<unit::base>>&
	{
		return at_impl(*this, p);
	}
	auto at(vector<std::size_t> p) const
		-> const std::vector<std::shared_ptr<unit::base>>&
	{
		return at_impl(*this, p);
	}

private:
	template <typename U>
	static auto at_impl(U& u, vector<std::size_t> p) -> decltype(u.at(p))
	{
		return u.m_voxels.at(p.x).at(p.y);
	}

	std::vector<std::vector<std::vector<std::shared_ptr<unit::base>>>> m_voxels{
		1,
		std::vector<std::vector<std::shared_ptr<unit::base>>>{1}};
	vector<double> m_world_min{0.0, 0.0}, m_world_max{1.0, 1.0};
};

inline auto compute_index(const space_bins& bins, vector<double> pos)
	-> vector<std::size_t>
{
	let world_size = bins.world_max() - bins.world_min();
	let world_pos = pos - bins.world_min();
	let world_dim = bins.dimensions_in_voxels();
	return vector<std::size_t>{
		max(vector<double>{0.0, 0.0},
			vector<double>{(world_pos.x / world_size.x) * world_dim.x,
						   (world_pos.y / world_size.y) * world_dim.y})};
}

template <typename It>
void divide_space(It first_unit, It last_unit, space_bins& bins)
{
	let world_dim = bins.dimensions_in_voxels();
	for (auto ind : range(world_dim)) {
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

template <typename unit_type = unit::base, typename T>
auto find(const space_bins& bins, vector<double> pos, double radius, T pred)
{
	std::vector<std::shared_ptr<unit_type>> ret;

	for (auto ind : range(
			 compute_index(bins, pos - vector<double>{radius, radius}),
			 min(bins.dimensions_in_voxels(),
				 vector<std::size_t>{1, 1} +
					 compute_index(
						 bins, pos + vector<double>{radius, radius})))) {
		for (let& unit : bins.at(ind)) {
			if (auto ptr =
					check_proximity<unit_type>(unit, pos, radius, pred)) {
				ret.emplace_back(ptr);
			}
		}
	}

	return ret;
}

template <typename unit_type = unit::base, typename T>
auto find_one(const space_bins& bins, vector<double> pos, double radius, T pred)
	-> std::shared_ptr<unit_type>
{
	for (auto ind : range(
			 compute_index(bins, pos - vector<double>{radius, radius}),
			 min(bins.dimensions_in_voxels(),
				 vector<std::size_t>{1, 1} +
					 compute_index(
						 bins, pos + vector<double>{radius, radius})))) {
		for (let& unit : bins.at(ind)) {
			if (auto ptr =
					check_proximity<unit_type>(unit, pos, radius, pred)) {
				return ptr;
			}
		}
	}

	return {};
}

} // namespace mark

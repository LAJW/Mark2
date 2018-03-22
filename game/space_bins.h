#pragma once

#include <unit/base.h>

namespace mark {

template <typename T>
class space_bins final {
public:
	space_bins() = default;

	space_bins(
		vector<std::size_t> dimensions_in_voxels,
		vector<double> world_min,
		vector<double> world_max)
		: m_voxels{dimensions_in_voxels.x,
				   std::vector<T>{dimensions_in_voxels.y}}
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

	auto at(vector<std::size_t> p) -> T& { return at_impl(*this, p); }
	auto at(vector<std::size_t> p) const -> const T&
	{
		return at_impl(*this, p);
	}

private:
	template <typename U>
	static auto at_impl(U& u, vector<std::size_t> p) -> decltype(u.at(p))
	{
		return u.m_voxels.at(p.x).at(p.y);
	}

	std::vector<std::vector<T>> m_voxels{1, std::vector<T>{1}};
	vector<double> m_world_min{0.0, 0.0}, m_world_max{1.0, 1.0};
};

template <typename T>
auto compute_index(const space_bins<T>& bins, vector<double> pos)
	-> vector<std::size_t>
{
	let world_size = bins.world_max() - bins.world_min();
	let world_pos = pos - bins.world_min();
	let world_dim = bins.dimensions_in_voxels();
	return {static_cast<std::size_t>(
				std::max(0.0, (world_pos.x / world_size.x) * world_dim.x)),
			static_cast<std::size_t>(
				std::max(0.0, (world_pos.y / world_size.y) * world_dim.y))};
}

template <typename It>
void divide_space(
	It first_unit,
	It last_unit,
	space_bins<std::vector<std::shared_ptr<unit::base>>>& bins)
{
	let world_dim = bins.dimensions_in_voxels();
	for (std::size_t x = 0; x != world_dim.x; ++x) {
		for (std::size_t y = 0; y != world_dim.y; ++y) {
			bins.at({x, y}).clear();
		}
	}

	std::for_each(first_unit, last_unit, [&bins](let& unit) {
		bins.at(compute_index(bins, unit->pos())).emplace_back(unit);
	});
}

template <typename unit_type = unit::base, typename T>
auto find(
	const space_bins<std::vector<std::shared_ptr<unit::base>>>& bins,
	vector<double> pos,
	double radius,
	T pred) -> std::vector<std::shared_ptr<unit_type>>
{
	std::vector<std::shared_ptr<unit_type>> ret;

	let min_bin = compute_index(bins, pos - vector<double>{radius, radius});
	let max_bin = compute_index(bins, pos + vector<double>{radius, radius});
	let world_dim = bins.dimensions_in_voxels();

	// mmmmmm beautiful
	for (std::size_t x = min_bin.x,
					 x_max = std::min(max_bin.x + 1, world_dim.x);
		 x < x_max;
		 ++x) {
		for (std::size_t y = min_bin.y,
						 y_max = std::min(max_bin.y + 1, world_dim.y);
			 y < y_max;
			 ++y) {
			for (let& unit : bins.at({x, y})) {
				if (length(unit->pos() - pos) < radius && pred(*unit)) {
					if (let& derived =
							std::dynamic_pointer_cast<unit_type>(unit)) {
						ret.emplace_back(derived);
					}
				}
			}
		}
	}

	return ret;
}

template <typename unit_type = unit::base, typename T>
auto find_one(
	const space_bins<std::vector<std::shared_ptr<unit::base>>>& bins,
	vector<double> pos,
	double radius,
	T pred) -> std::shared_ptr<unit_type>
{
	let min_bin = compute_index(bins, pos - vector<double>{radius, radius});
	let max_bin = compute_index(bins, pos + vector<double>{radius, radius});
	let world_dim = bins.dimensions_in_voxels();

	// mmmmmm beautiful
	for (std::size_t x = min_bin.x,
					 x_max = std::min(max_bin.x + 1, world_dim.x);
		 x < x_max;
		 ++x) {
		for (std::size_t y = min_bin.y,
						 y_max = std::min(max_bin.y + 1, world_dim.y);
			 y < y_max;
			 ++y) {
			for (let& unit : bins.at({x, y})) {
				if (length(unit->pos() - pos) < radius && pred(*unit)) {
					if (let& derived =
							std::dynamic_pointer_cast<unit_type>(unit)) {
						return derived;
					}
				}
			}
		}
	}

	return {};
}

} // namespace mark

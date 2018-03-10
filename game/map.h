#pragma once
#include "stdafx.h"

namespace mark {
class map final {
public:
	static constexpr const double tile_size = 64.0;

	static map make_cavern(resource::manager&);
	static map make_square(resource::manager&);

	map(resource::manager&, const YAML::Node&);

	auto traversable(const vector<double>& pos, double radius) const -> bool;
	auto traversable(
		const vector<int>& i_pos, const size_t radius) const -> bool;

	void tick(
		vector<double> top_left,
		vector<double> bottom_right,
		tick_context& context);

	auto find_path(
		const vector<double>& start,
		const vector<double>& end,
		double radius = 0.0) const->std::vector<vector<double>>;

	// Can find be called in this tick (limit find count per frame)
	auto can_find() const -> bool;

	auto collide(const segment_t&) const -> std::optional<vector<double>>;

	void serialise(YAML::Emitter&) const;

	// Not a part of the public interface
	struct Node {
		vector<int> pos;
		int f = 0; // distance from starting + distance from ending (h)
		const Node* parent = nullptr;
	};
	auto map_to_world(const vector<int>&) const noexcept
		-> vector<double>;
private:
	struct terrain {
		enum class type {
			null,
			abyss,
			floor_1,
			floor_2,
			floor_3,
			wall
		} type = type::null;
		std::array<bool, 20> traversable;
		unsigned variant = 0;
	};
	auto p_traversable(
		const vector<int>& i_pos, const size_t radius) const -> bool;

	static std::string serialize_terrain_type(enum terrain::type);
	static enum terrain::type deserialize_terrain_type(const std::string&);

	map(resource::manager&, const vector<size_t>& size);

	auto size() const noexcept -> const vector<size_t>&;
	auto get(const vector<int>& pos) const noexcept -> enum terrain::type;
	auto get_variant(const vector<int>& pos) const noexcept -> unsigned;
	void set(const vector<int>& pos, enum terrain::type) noexcept;
	auto world_to_map(const vector<double>&) const noexcept
		-> vector<int>;
	void calculate_traversable();

	std::vector<terrain> m_terrain;
	std::reference_wrapper<resource::manager> m_rm;
	vector<size_t> m_size;
	mutable unsigned m_find_count = 0;
};
}

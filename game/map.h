#pragma once
#include "stdafx.h"
#include "vector.h"

namespace mark {
namespace resource {
	class manager;
}
struct sprite;
class world;
struct tick_context;

class map final {
public:
	static constexpr const double tile_size = 32.0;

	static map make_cavern(resource::manager&);
	static map make_square(resource::manager&);

	map(resource::manager&, const YAML::Node&);

	auto traversable(const vector<double>& pos, double radius) const -> bool;
	auto map::traversable(
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

	auto collide(segment_t) const
		-> std::optional<vector<double>>;

	void serialize(YAML::Emitter&) const;

	// Not a part of the public interface
	struct Node {
		vector<int> pos;
		int f = 0; // distance from starting + distance from ending (h)
		const Node* parent = nullptr;
	};
	auto map_to_world(vector<int>) const noexcept
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
	auto map::p_traversable(
		const vector<int>& i_pos, const size_t radius) const -> bool;

	static std::string serialize_terrain_type(enum class terrain::type);
	static enum class terrain::type deserialize_terrain_type(const std::string&);

	map(resource::manager&, vector<size_t> size);

	auto size() const noexcept -> const vector<size_t>&;
	auto get(vector<int> pos) const noexcept -> enum class terrain::type;
	auto get_variant(vector<int> pos) const noexcept -> unsigned;
	void set(vector<int> pos, enum class terrain::type) noexcept;
	auto world_to_map(vector<double>) const noexcept
		-> vector<int>;
	void calculate_traversable();

	std::vector<terrain> m_terrain;
	std::reference_wrapper<resource::manager> m_rm;
	vector<size_t> m_size;
	mutable unsigned m_find_count = 0;
};
}
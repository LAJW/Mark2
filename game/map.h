#pragma once
#include "stdafx.h"
#include "interface_serializable.h"
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

	static mark::map make_cavern(mark::resource::manager&);
	static mark::map make_square(mark::resource::manager&);

	map(mark::resource::manager&, const YAML::Node&);

	auto traversable(const vector<double>& pos, double radius) const -> bool;
	auto mark::map::traversable(
		const vector<int>& i_pos, const size_t radius) const -> bool;

	void tick(
		mark::vector<double> top_left,
		mark::vector<double> bottom_right,
		mark::tick_context& context);

	auto find_path(
		const mark::vector<double>& start,
		const mark::vector<double>& end,
		double radius = 0.0) const->std::vector<mark::vector<double>>;

	// Can find be called in this tick (limit find count per frame)
	auto can_find() const -> bool;

	auto collide(mark::segment_t) const
		-> std::optional<mark::vector<double>>;

	void serialize(YAML::Emitter&) const;

	// Not a part of the public interface
	struct Node {
		mark::vector<int> pos;
		int f = 0; // distance from starting + distance from ending (h)
		const Node* parent = nullptr;
	};
	auto map_to_world(mark::vector<int>) const noexcept
		-> mark::vector<double>;
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
	auto mark::map::p_traversable(
		const vector<int>& i_pos, const size_t radius) const -> bool;

	static std::string serialize_terrain_type(enum class terrain::type);
	static enum class terrain::type deserialize_terrain_type(const std::string&);

	map(mark::resource::manager&, mark::vector<size_t> size);

	auto size() const noexcept -> const mark::vector<size_t>&;
	auto get(mark::vector<int> pos) const noexcept -> enum class terrain::type;
	auto get_variant(mark::vector<int> pos) const noexcept -> unsigned;
	void set(mark::vector<int> pos, enum class terrain::type) noexcept;
	auto world_to_map(mark::vector<double>) const noexcept
		-> mark::vector<int>;
	void calculate_traversable();

	std::vector<terrain> m_terrain;
	std::reference_wrapper<mark::resource::manager> m_rm;
	mark::vector<size_t> m_size;
	mutable unsigned m_find_count = 0;
};
}
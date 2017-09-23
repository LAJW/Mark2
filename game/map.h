#pragma once
#include "stdafx.h"
#include "iserializable.h"
#include "vector.h"

namespace mark {
	namespace resource {
		class manager;
	}
	class sprite;
	class world;
	struct tick_context;

	class map final {
	public:
		static constexpr const double tile_size = 32.0;

		static mark::map make_cavern(mark::resource::manager&);
		static mark::map make_square(mark::resource::manager&);

		map(mark::resource::manager&, const YAML::Node&);

		auto traversable(
			mark::vector<double> pos,
			double radius = 0.0) const -> bool;

		void tick(
			mark::vector<double> top_left,
			mark::vector<double> bottom_right,
			mark::tick_context& context);

		auto find_path(
			mark::vector<double> start,
			mark::vector<double> end,
			double radius = 0.0) const->std::vector<mark::vector<double>>;

		// Can find be called in this tick (limit find count per frame)
		auto can_find() const -> bool;

		auto collide(mark::segment_t) const
			-> std::optional<mark::vector<double>>;

		void serialize(YAML::Emitter&) const;

	private:
		enum class terrain_type {
			null,
			abyss,
			floor_1,
			floor_2,
			floor_3,
			wall
		};

		static std::string serialize_terrain_type(terrain_type);
		static terrain_type deserialize_terrain_type(const std::string&);

		map(mark::resource::manager&, mark::vector<size_t> size);
		auto mark::map::traversable(
			const mark::vector<int> i_pos, const size_t radius) const -> bool;

		auto size() const noexcept -> const mark::vector<size_t>&;
		auto get(mark::vector<int> pos) const noexcept -> terrain_type;
		auto get_variant(mark::vector<int> pos) const noexcept -> unsigned;
		void set(mark::vector<int> pos, terrain_type) noexcept;
		auto world_to_map(mark::vector<double>) const noexcept
			-> mark::vector<int>;
		auto map_to_world(mark::vector<int>) const noexcept
			-> mark::vector<double>;

		std::reference_wrapper<mark::resource::manager> m_rm;
		std::vector<terrain_type> m_terrain;
		std::vector<unsigned> m_variant;
		mark::vector<size_t> m_size;
		mutable unsigned m_find_count = 0;
	};
}
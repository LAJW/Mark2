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

	class map final : public mark::iserializable {
	public:
		static constexpr const double tile_size = 32.0;

		static mark::map make_cavern(mark::resource::manager& resource_manager);
		static mark::map make_square(mark::resource::manager& resource_manager);

		map(mark::resource::manager&, const YAML::Node&);

		auto traversable(mark::vector<double> pos, double radius = 0.0) const -> bool;

		void tick(mark::vector<double> top_left, mark::vector<double> bottom_right, mark::tick_context& context);

		auto find_path(
			mark::vector<double> start,
			mark::vector<double> end,
			double radius = 0.0) const->std::vector<mark::vector<double>>;

		// Can find be called in this tick (limit find count per frame)
		auto can_find() const -> bool;

		// collide world with a line segment return NAN/NAN if there was no collision
		auto collide(mark::segment_t) const -> mark::vector<double>;

		void serialize(YAML::Emitter&) const override;

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
		static terrain_type deserialize(const std::string&);

		map(mark::resource::manager&, mark::vector<size_t> size);
		auto mark::map::traversable(const mark::vector<int> i_pos, const size_t radius) const -> bool;

		auto size() const noexcept -> const mark::vector<size_t>&;
		auto at(mark::vector<int> pos) noexcept -> terrain_type*;
		auto at(mark::vector<int> pos) const noexcept -> const terrain_type*;
		auto world_to_map(mark::vector<double>) const noexcept->mark::vector<int>;
		auto map_to_world(mark::vector<int>) const noexcept->mark::vector<double>;

		std::reference_wrapper<mark::resource::manager> m_rm;
		std::vector<terrain_type> m_terrain;
		mark::vector<size_t> m_size;
		mutable unsigned m_find_count = 0;
	};
}
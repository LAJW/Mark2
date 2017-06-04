#pragma once
#include "stdafx.h"
#include "iserializable.h"
#include "vector.h"

namespace mark {
	namespace resource {
		class manager;
	}
	namespace terrain {
		class base;
	};
	class sprite;
	class world;
	struct tick_context;

	class map final : public mark::iserializable {
	public:
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
		using terrain_t = std::vector<std::vector<std::shared_ptr<mark::terrain::base>>>;

		map(mark::map::terrain_t data);

		auto size() const -> mark::vector<int>;

		auto traversable(mark::vector<int> pos, int radius) const -> bool;

		terrain_t m_terrain;
		mutable unsigned m_find_count;
	};
}
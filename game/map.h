﻿#pragma once
#include "stdafx.h"
#include <vector2d.h>

namespace mark {
class map final
{
public:
	static constexpr const double tile_size = 64.0;

	static map make_cavern(resource::manager&);
	static map make_square(resource::manager&);

	map(resource::manager&, const YAML::Node&);

	auto traversable(const vector<double>& pos, double radius) const -> bool;
	auto traversable(const vector<int>& i_pos, const size_t radius) const
		-> bool;

	void update(
		vector<double> top_left,
		vector<double> bottom_right,
		update_context& context);

	auto find_path(
		const vector<double>& start,
		const vector<double>& end,
		double radius = 0.0) const -> std::vector<vector<double>>;

	// Can find be called in this update (limit find count per frame)
	auto can_find() const -> bool;

	auto collide(const segment_t&) const -> std::optional<collide_result>;

	void serialize(YAML::Emitter&) const;

	// Not a part of the public interface
	struct Node
	{
		vector<int> pos;
		int f = 0; // distance from starting + distance from ending (h)
		const Node* parent = nullptr;
	};
	auto map_to_world(const vector<int>&) const noexcept -> vector<double>;

	auto size() const noexcept -> const vector<size_t>&;

private:
	enum class terrain_kind
	{
		null,
		abyss,
		floor_1,
		floor_2,
		floor_3,
		wall
	};
	struct terrain
	{
		terrain_kind type = terrain_kind::null;
		std::array<bool, 20> traversable;
		unsigned variant = 0;
	};
	auto p_traversable(const vector<int>& i_pos, const size_t radius) const
		-> bool;

	static std::string serialize_terrain_kind(terrain_kind);
	static enum terrain_kind deserialize_terrain_kind(const std::string&);

	map(resource::manager&, const vector<size_t>& size);

	auto get(const vector<int>& pos) const noexcept -> terrain_kind;
	auto get_variant(const vector<int>& pos) const noexcept -> unsigned;
	void set(const vector<int>& pos, terrain_kind) noexcept;
	auto world_to_map(const vector<double>&) const noexcept -> vector<int>;
	auto collide_with_block_at(vector<double> pos, segment_t segment) const
		noexcept -> std::optional<collide_result>;
	void calculate_traversable();

	static constexpr size_t map_size = 1000;
	vector2d<terrain> m_terrain;
	std::shared_ptr<const resource::image> m_tileset;
	mutable unsigned m_find_count = 0;
};
} // namespace mark

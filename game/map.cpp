﻿#include "map.h"
#include "algorithm.h"
#include "base64.h"
#include "exception.h"
#include "resource_manager.h"
#include "sprite.h"
#include "stdafx.h"
#include "update_context.h"
#include "vector.h"

// Make specific types of maps

using Node = mark::map::Node;

mark::map mark::map::make_cavern(mark::resource::manager& rm)
{
	let constexpr map_size = 1000ull;
	let depth = 100;
	let max_corridor_length = 10;
	let spacing = 5;
	let corridor_width = 3;

	auto map = mark::map(rm, { map_size, map_size });
	auto point = vector<int>(map.size()) / 2;
	for (let cur_depth : range(depth)) {
		(void)cur_depth;
		let direction_d = rotate(vector<double>(1, 0), rm.random(0, 3) * 90.);
		let direction = round(direction_d);
		let orto = round(rotate(direction_d, 90.));
		let length = rm.random(1, max_corridor_length);

		for (let cur_spacing : range(-spacing, length + spacing)) {
			for (let orto_offset : range(-corridor_width, corridor_width + 1)) {
				map.set(
					point + direction * cur_spacing + orto * orto_offset,
					terrain_kind::floor_1);
			}
		}
		point += direction * length;
	}
	map.calculate_traversable();
	return map;
}

mark::map mark::map::make_square(resource::manager& resource_manager)
{
	let constexpr square_size = 20ull;
	auto map = mark::map(resource_manager, { square_size, square_size });
	let middle = range(vector<size_t>(1, 1), map.size() - vector<size_t>(1, 1));
	for (let pos : middle) {
		map.set(vector<int>(pos), terrain_kind::floor_1);
	}
	map.calculate_traversable();
	return map;
}

// Map functionality

auto mark::map::world_to_map(const vector<double>& pos) const noexcept
	-> vector<int>
{
	return round(pos / map::tile_size) + vector<int>(this->size() / size_t(2));
}

void mark::map::calculate_traversable()
{
	for (let pos : range(m_size)) {
		auto& traversables = m_terrain[pos.x + pos.y * m_size.x].traversable;
		traversables.front() = p_traversable(vector<int>(pos), 0);
		for (let radius : range<size_t>(1, traversables.size())) {
			traversables[radius] = traversables[radius - 1]
				&& p_traversable(vector<int>(pos), radius);
		}
	}
}

auto mark::map::map_to_world(const vector<int>& pos) const noexcept
	-> vector<double>
{
	let center = vector<int>(this->size() / size_t(2));
	return vector<double>(pos - center) * map::tile_size;
}

auto mark::map::traversable(const vector<double>& pos, const double radius_)
	const -> bool
{
	let radius = size_t(std::ceil(radius_ / map::tile_size));
	return this->traversable(this->world_to_map(pos), radius);
}

auto mark::map::traversable(const vector<int>& pos, const size_t radius) const
	-> bool
{
	return pos.x >= 0 && pos.x < (int)m_size.x && pos.y >= 0
		&& pos.y < (int)m_size.y && radius < 20
		&& m_terrain[pos.x + m_size.x * pos.y].traversable[radius];
}

auto mark::map::p_traversable(const vector<int>& pos, const size_t uradius)
	const -> bool
{
	if (uradius <= 1) {
		let tile = this->get(pos);
		return tile != terrain_kind::null && tile != terrain_kind::wall;
	}
	let radius = static_cast<int>(uradius);
	let offset = vector<int>(radius, radius);
	let range = mark::range(-offset, offset);
	return std::all_of(range.begin(), range.end(), [&](let offset) {
		if (length(offset) > radius) {
			return true;
		}
		let tile = this->get(pos + offset);
		return tile != terrain_kind::null && tile != terrain_kind::wall;
	});
}

void mark::map::update(
	vector<double> world_tl,
	vector<double> world_br,
	update_context& context)
{
	m_find_count = 0;
	let range = [&] {
		let size = vector<int>(this->size());
		let tl_ = this->world_to_map(world_tl);
		let br_ = this->world_to_map(world_br);
		let tl = vector<int>(std::max(tl_.x, 0), std::max(tl_.y, 0));
		let br = vector<int>(std::min(br_.x, size.x), std::min(br_.y, size.y));
		return mark::range(tl, br);
	}();
	transform(
		range.begin(),
		range.end(),
		back_inserter(context.sprites[-1]),
		[&](let pos) {
			let frame = [&] {
				let ctl = this->get(pos - vector<int>(1, 1));
				let cbl = this->get(pos - vector<int>(1, 0));
				let ctr = this->get(pos - vector<int>(0, 1));
				let cbr = this->get(pos);
				return ((ctl == terrain_kind::floor_1) & 1)
					| ((cbl == terrain_kind::floor_1) & 1) << 1
					| ((ctr == terrain_kind::floor_1) & 1) << 2
					| ((cbr == terrain_kind::floor_1) & 1) << 3;
			}();
			sprite info;
			info.image = m_tileset;
			info.frame = frame + get_variant(pos) * 16;
			info.size = map::tile_size;
			info.pos = map_to_world(pos) - vector<double>(0.5, 0.5) * tile_size;
			return info;
		});
}

std::string mark::map::serialize_terrain_kind(terrain_kind t)
{
	switch (t) {
	case terrain_kind::null:
		return "null";
	case terrain_kind::abyss:
		return "abyss";
	case terrain_kind::floor_1:
		return "floor_1";
	case terrain_kind::floor_2:
		return "floor_2";
	case terrain_kind::floor_3:
		return "floor_3";
	case terrain_kind::wall:
		return "wall";
	default:
		throw std::bad_cast();
	}
}

auto mark::map::deserialize_terrain_kind(const std::string& str)
	-> map::terrain_kind
{
	if (str == "null")
		return terrain_kind::null;
	if (str == "abyss")
		return terrain_kind::abyss;
	if (str == "floor_1")
		return terrain_kind::floor_1;
	if (str == "floor_2")
		return terrain_kind::floor_2;
	if (str == "floor_3")
		return terrain_kind::floor_3;
	if (str == "wall")
		return terrain_kind::wall;
	throw std::bad_cast();
}

mark::map::map(resource::manager& rm, const vector<size_t>& size)
	: m_tileset(rm.image("jungle-1.png"))
	, m_size(size)
	, m_terrain(size.x * size.y)
{
	for (auto& terrain : m_terrain) {
		terrain.variant = rm.random<unsigned>(0, 2);
	}
}

auto mark::map::size() const noexcept -> const vector<size_t>&
{
	return m_size;
}

auto mark::map::get(const vector<int>& pos) const noexcept -> terrain_kind
{
	if (pos.x >= 0 && pos.x < (int)m_size.x && pos.y >= 0
		&& pos.y < (int)m_size.y) {
		return m_terrain[pos.x + m_size.x * pos.y].type;
	}
	return terrain_kind::null;
}

auto mark::map::get_variant(const vector<int>& pos) const noexcept -> unsigned
{
	if (pos.x >= 0 && pos.x < (int)m_size.x && pos.y >= 0
		&& pos.y < (int)m_size.y) {
		return m_terrain[pos.x + m_size.x * pos.y].variant;
	}
	return 0;
}

void mark::map::set(const vector<int>& pos, terrain_kind type) noexcept
{
	if (pos.x >= 0 && pos.x < (int)m_size.x && pos.y >= 0
		&& pos.y < (int)m_size.y) {
		m_terrain[pos.x + m_size.x * pos.y].type = type;
	}
}

// 16-point star
std::array<mark::vector<int>, 16> directions{
	mark::vector<int>{ -2, -1 },
	{ -1, -1 },
	{ -1, -2 },
	{ 2, -1 },
	{ 1, -1 },
	{ 1, -2 },
	{ -2, 1 },
	{ -1, 1 },
	{ -1, 2 },
	{ 2, 1 },
	{ 1, 1 },
	{ 1, 2 },
	{ 1, 0 },
	{ 0, 1 },
	{ -1, 0 },
	{ 0, -1 },
};

auto nearest_traversable(
	const mark::map& map,
	const mark::vector<int>& end,
	const size_t radius) -> mark::vector<int>
{
	if (!map.traversable(end, radius)) {
		for (let i : mark::range(100)) {
			for (let neighbour : directions) {
				let cur = neighbour * i + end;
				if (map.traversable(cur, radius)) {
					return cur;
				}
			}
		}
	}
	return end;
}

auto nearest_traversable(
	const mark::map& map,
	const mark::vector<int>& start,
	const mark::vector<int>& end,
	const size_t radius) -> mark::vector<int>
{
	using namespace mark;
	if (!map.traversable(end, radius)) {
		for (let i : range(100)) {
			for (let neighbour : directions) {
				let cur = neighbour * i + end;
				if (map.traversable(cur, radius)) {
					if (length(cur - end) <= length(start - end)) {
						return cur;
					}
				}
			}
		}
	}
	return end;
}

static auto straight_exists(
	const mark::map& map,
	const mark::vector<double>& start,
	const mark::vector<double>& end,
	const double radius)
{
	using namespace mark;
	let diff = end - start;
	let dir = normalize(diff);
	let dist = length(diff);
	for (double i = 0.0; i <= dist; i += map::tile_size / 2.f) {
		if (!map.traversable(dir * i + start, radius)) {
			return false;
		}
	}
	return true;
}

static bool
has_one(const std::vector<std::unique_ptr<Node>>& nodes, mark::vector<int> pos)
{
	return any_of(nodes.begin(), nodes.end(), [&pos](let& node) {
		return node->pos == pos;
	});
}

static auto find_one(std::vector<Node>& nodes, const mark::vector<int>& pos)
	-> Node*
{
	auto node_it = find_if(nodes.begin(), nodes.end(), [&pos](let& node) {
		return pos == node.pos;
	});
	return node_it == nodes.end() ? nullptr : &(*node_it);
}

static auto make_path(
	const mark::map& map,
	const Node* node,
	std::vector<mark::vector<double>> out = {})
	-> std::vector<mark::vector<double>>
{
	if (node) {
		out.push_back(map.map_to_world(node->pos));
		return make_path(map, node->parent, move(out));
	}
	return out;
}

// Jump Point Search Functions - Beginning

static auto is_diagonal(const mark::vector<int>& direction)
{
	return direction.x != 0 && direction.y != 0;
}

static auto turn_left(const mark::vector<int>& direction) -> mark::vector<int>
{
	return { -direction.y, direction.x };
}

static auto turn_right(const mark::vector<int>& direction) -> mark::vector<int>
{
	return { direction.y, -direction.x };
}

static auto has_forced_neighbours(
	const mark::map& map,
	const mark::vector<int>& next,
	const mark::vector<int>& direction,
	const int radius) -> bool
{
	if (!is_diagonal(direction)) {
		let p1 = !map.traversable(next + turn_left(direction), radius);
		let p2 =
			map.traversable(next + turn_left(direction) + direction, radius);
		let p3 = !map.traversable(next + turn_right(direction), radius);
		let p4 =
			map.traversable(next + turn_right(direction) + direction, radius);
	}
	return !is_diagonal(direction)
		&& ((!map.traversable(next + turn_left(direction), radius)
			 && map.traversable(
					next + turn_left(direction) + direction, radius))
			|| (!map.traversable(next + turn_right(direction), radius)
				&& map.traversable(
					   next + turn_right(direction) + direction, radius)));
}

static auto make_direction(const Node& node) -> mark::vector<int>
{
	if (node.parent) {
		auto direction = node.pos - node.parent->pos;
		direction.x = mark::sgn(direction.x);
		direction.y = mark::sgn(direction.y);
		return direction;
	}
	return { 0, 0 };
}

static auto
pruned_neighbours(const mark::map& map, const Node& node, const int radius)
	-> std::vector<mark::vector<int>>
{
	std::vector<mark::vector<int>> out;
	let direction = make_direction(node);
	if (direction.x == 0 && direction.y == 0) {
		for (let& next_direction : directions) {
			if (map.traversable(next_direction + node.pos, radius)) {
				out.push_back(next_direction + node.pos);
			}
		}
		return out;
	}
	if (is_diagonal(direction)) {
		let right = node.pos + mark::vector<int>(direction.x, 0);
		if (map.traversable(right, radius)) {
			out.push_back(right);
		}
		let top = node.pos + mark::vector<int>(0, direction.y);
		if (map.traversable(top, radius)) {
			out.push_back(top);
		}
		let edge = node.pos + direction;
		if (map.traversable(edge, radius)) {
			out.push_back(edge);
		}
		return out;
	}
	if (map.traversable(node.pos + direction, radius)) {
		out.push_back(node.pos + direction);
		if (!map.traversable(node.pos + turn_left(direction), radius)
			&& map.traversable(
				   node.pos + turn_left(direction) + direction, radius)) {
			out.push_back(node.pos + turn_left(direction) + direction);
		}
		if (!map.traversable(node.pos + turn_right(direction), radius)
			&& map.traversable(
				   node.pos + turn_right(direction) + direction, radius)) {
			out.push_back(node.pos + turn_right(direction) + direction);
		}
		return out;
	}
	return {};
}

static auto jump(
	const mark::map& map,
	const mark::vector<int>& cur,
	const mark::vector<int>& direction,
	const mark::vector<int>& end,
	const int radius) -> std::optional<mark::vector<int>>
{
	let next = cur + direction;
	if (!map.traversable(next, radius)) {
		return {};
	}
	if (next == end) {
		return next;
	}
	if (has_forced_neighbours(map, next, direction, radius)) {
		return next;
	}
	if (is_diagonal(direction)
		&& (jump(map, next, { direction.x, 0 }, end, radius)
			|| jump(map, next, { 0, direction.y }, end, radius))) {
		return next;
	}
	return jump(map, next, direction, end, radius);
}

static auto identify_successors(
	const mark::map& map,
	const Node& cur,
	const mark::vector<int>& end,
	const int radius) -> std::vector<Node>
{
	std::vector<Node> successors;
	for (let& neighbour : pruned_neighbours(map, cur, radius)) {
		let pos = jump(map, cur.pos, neighbour - cur.pos, end, radius);
		if (pos) {
			Node node;
			node.parent = &cur;
			node.pos = *pos;
			node.f = cur.f + static_cast<int>(mark::length(*pos - cur.pos));
			successors.push_back(node);
		}
	}
	return successors;
}

// Jump Point Search Functions - End

static auto find_path(
	const mark::map& map,
	const mark::vector<int>& end,
	const int radius,
	std::vector<Node> open,
	std::vector<std::unique_ptr<Node>> closed)
	-> std::vector<mark::vector<double>>
{
	if (open.empty()) {
		return {};
	}
	let open_min_it = min_element(
		open.begin(), open.end(), [](let& n1, let& n2) { return n1.f < n2.f; });

	closed.push_back(std::make_unique<Node>(*open_min_it));
	auto& current = *closed.back();
	open.erase(open_min_it);

	if (current.pos == end) {
		return make_path(map, &current);
	}

	let successors = identify_successors(map, current, end, radius);
	open = accumulate(
		successors.begin(),
		successors.end(),
		move(open),
		[&](auto& open, let& successor) {
			auto neighbour_pos = successor.pos;
			if (has_one(closed, neighbour_pos)) {
				return open;
			}
			let neighbour = find_one(open, neighbour_pos);
			if (!neighbour) {
				open.push_back(successor);
			} else if (neighbour->f > successor.f) {
				neighbour->f = successor.f;
				neighbour->parent = successor.parent;
			}
			return move(open);
		});
	return find_path(map, end, radius, move(open), move(closed));
}

auto mark::map::find_path(
	const vector<double>& world_start,
	const vector<double>& world_end,
	const double world_radius) const -> std::vector<vector<double>>
{
	if (straight_exists(*this, world_start, world_end, world_radius)) {
		return { world_end };
	}
	let radius = static_cast<int>(std::ceil(world_radius / map::tile_size));
	let start = world_to_map(world_start);
	// Start not traversable - get as quickly to the world as possible
	if (!this->traversable(start, radius)) {
		let end = ::nearest_traversable(*this, start, radius);
		return { this->map_to_world(end) };
	} else {
		let end = ::nearest_traversable(
			*this, start, world_to_map(world_end), radius);
		let f = static_cast<int>(length(end - start));
		m_find_count++;
		return ::find_path(
			*this, end, radius, { Node{ start, f, nullptr } }, {});
	}
}

auto mark::map::can_find() const -> bool { return m_find_count <= 5; }

// clang-format off
const std::array<mark::segment_t, 4> square {
	mark::segment_t{ { -1, -1 }, { -1, 1 } },
	{ { -1, 1 }, { 1, 1 } },
	{ { 1, 1 }, { 1, -1 } },
	{ { 1, -1 }, { -1, -1 } }
};
// clang-format on

static auto intersect_and_reflect(mark::segment_t wall, mark::segment_t ray)
	-> std::optional<mark::collide_result>
{
	using namespace mark;
	if (let intersection = intersect(ray, wall)) {
		let horizontal = (wall.first.x == wall.second.x);
		let dir = ray.second - ray.first;
		let reflection = horizontal ? atan(vector<double>(-dir.x, dir.y))
									: atan(vector<double>(dir.x, -dir.y));
		let reflection_f = static_cast<float>(reflection);
		return collide_result{ *intersection, reflection_f };
	}
	return {};
}
auto mark::map::collide_with_block_at(vector<double> pos, segment_t ray) const
	noexcept -> std::optional<collide_result>
{
	let constexpr hs = mark::map::tile_size / 2.; // Half size (of the map tile)
	let cur_pos = this->world_to_map(pos);
	let cur_kind = this->get(cur_pos);
	if (cur_kind == terrain_kind::floor_1) {
		return {};
	}
	let center = this->map_to_world(cur_pos);
	std::vector<collide_result> collisions;
	for (let& wall : square) {
		let extended_wall = segment_t(wall.first * hs, wall.second * hs);
		let offset_wall = segment_t(
			extended_wall.first + center, extended_wall.second + center);
		if (let intersection = intersect_and_reflect(offset_wall, ray)) {
			collisions.push_back(*intersection);
		}
	}
	let origin = ray.first;
	return min_element_v(collisions, [&](let& a, let& b) {
		return length(a.pos - origin) < length(b.pos - origin);
	});
}

auto mark::map::collide(const segment_t& segment) const
	-> std::optional<collide_result>
{
	let constexpr hs = mark::map::tile_size / 2.; // Half size (of the map tile)
	let direction = normalize(segment.second - segment.first);
	let length = mark::length(segment.second - segment.first);
	for (double i = hs; i < length + hs; i += hs) {
		if (let collision = this->collide_with_block_at(
				segment.first + i * direction, segment)) {
			return *collision;
		}
	}
	return {};
}

void mark::map::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << "map";
	out << Key << "size" << Value << BeginMap;
	out << Key << "x" << Value << m_size.x;
	out << Key << "y" << Value << m_size.y;
	out << EndMap;
	std::string data;
	data.reserve(m_terrain.size());
	for (let terrain : m_terrain) {
		data.push_back(static_cast<unsigned char>(terrain.type));
	}
	out << Key << "data" << Value << base64_encode(data);
	out << EndMap;
}

mark::map::map(resource::manager& resource_manager, const YAML::Node& node)
	: map(resource_manager, node["size"].as<vector<size_t>>())
{
	if (node["type"].as<std::string>() != "map") {
		std::runtime_error("BAD_DESERIALIZE");
	}
	m_terrain = { m_size.x * m_size.y, terrain() };
	size_t i = 0;
	auto data = base64_decode(node["data"].as<std::string>());
	for (let ch : data) {
		m_terrain[i].type = static_cast<terrain_kind>(ch);
		m_terrain[i].variant = resource_manager.random(0, 2);
		i++;
	}
	this->calculate_traversable();
}

#include "stdafx.h"
#include <algorithm>
#include <numeric>
#include <random>
#include "algorithm.h"
#include "map.h"
#include "resource_manager.h"
#include "vector.h"
#include "sprite.h"
#include "tick_context.h"
#include "exception.h"

// Make specific types of maps

using Node = mark::map::Node;

mark::map mark::map::make_cavern(mark::resource::manager& resource_manager) {
	mark::map map(resource_manager, { 1000, 1000 });
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<> dist_1_100(1, 10);
	std::uniform_int_distribution<> dist_0_3(0, 3);

	auto point = mark::vector<int>(500, 500);
	for (int i = 0; i < 100; i++) {
		const auto direction = mark::vector<int>(mark::rotate(mark::vector<float>(1, 0), dist_0_3(gen) * 90.f));
		const auto orto = mark::vector<int>(mark::rotate(mark::vector<float>(direction), 90.f));
		const auto length = dist_1_100(gen);

		for (int j = -5; j < length + 5; j++) {
			for (int k = -3; k <= 3; k++) {
				map.set(
					point + direction * j + orto * k,
					terrain_type::floor_1);
			}
		}
		point += direction * length;
	}
	map.calculate_traversable();
	return map;
}

mark::map mark::map::make_square(mark::resource::manager& resource_manager) {
	mark::map map(resource_manager, { 20, 20 });
	for (int x = 1; x < 20 - 1; x++) {
		for (int y = 1; y < 20 - 1; y++) {
			map.set({ x, y }, mark::map::terrain_type::floor_1);
		}
	}
	map.calculate_traversable();
	return map;
}

// Map functionality

auto mark::map::world_to_map(mark::vector<double> pos) const noexcept ->
	mark::vector<int> {
	return mark::round(pos / mark::map::tile_size)
		+ mark::vector<int>(this->size() / size_t(2));
}

void mark::map::calculate_traversable()
{
	for (const auto pos : range(m_size)) {
		auto& arr = m_traversable[pos.x + pos.y * m_size.x];
		bool traversable = true;
		for (const auto radius : range<size_t>(0, 20)) {
			// Avoid checking if block is traversable by an entity with that
			// radius, if it's known that it's not traversable by an entity
			// with smaller radius
			if (traversable) {
				traversable = p_traversable(vector<int>(pos), radius);
			}
			arr[radius] = traversable;
		}
	}
}

auto mark::map::map_to_world(mark::vector<int> pos) const noexcept ->
	mark::vector<double> {
	const auto center = mark::vector<int>(this->size() / size_t(2));
	return mark::vector<double>(pos - center) * mark::map::tile_size;
}

auto mark::map::traversable(
	const mark::vector<double>& pos,
	const double radius_) const -> bool {
	const auto radius = size_t(std::ceil(radius_ / mark::map::tile_size));
	return this->traversable(this->world_to_map(pos), radius);
}

auto mark::map::traversable(
	const mark::vector<int>& pos,
	const size_t radius) const -> bool
{
	return pos.x >= 0 && pos.x < m_size.x && pos.y >= 0 && pos.y < m_size.y
		&& radius < 20 && m_traversable[pos.x + m_size.x * pos.y][radius];
}

auto mark::map::p_traversable(
	const mark::vector<int>& i_pos,
	const size_t uradius) const -> bool {
	if (uradius > 1) {
		const auto radius = static_cast<int>(uradius);
		const auto offset = mark::vector<int>(radius, radius);
		for (const auto i : mark::range(-offset, offset)) {
			if (mark::length(i) <= radius) {
				const auto tile = this->get(i_pos + i);
				if (tile == terrain_type::null
					|| tile == terrain_type::wall) {
					return false;
				}
			}
		}
	} else {
		const auto tile = this->get(i_pos);
		if (tile == terrain_type::null
			|| tile == terrain_type::wall) {
			return false;
		}
	}
	return true;
}

void mark::map::tick(
	mark::vector<double> world_tl,
	mark::vector<double> world_br,
	mark::tick_context& context) {

	m_find_count = 0;
	const auto size = mark::vector<int>(this->size());
	const auto tl_ = this->world_to_map(world_tl);
	const auto br_ = this->world_to_map(world_br);
	const auto tl = mark::vector<int>(std::max(tl_.x, 0), std::max(tl_.y, 0));
	const auto br = mark::vector<int>(std::min(br_.x, size.x), std::min(br_.y, size.y));
	const auto floor = m_rm.get().image("ice-16.png");
	for (const auto pos : mark::range(tl, br)) {
		const auto ctl = this->get(pos - mark::vector<int>(1, 1));
		const auto cbl = this->get(pos - mark::vector<int>(1, 0));
		const auto ctr = this->get(pos - mark::vector<int>(0, 1));
		const auto cbr = this->get(pos);
		const auto frame =
			((ctl == terrain_type::floor_1) & 1)
			| ((cbl == terrain_type::floor_1) & 1) << 1
			| ((ctr == terrain_type::floor_1) & 1) << 2
			| ((cbr == terrain_type::floor_1) & 1) << 3;
		mark::sprite::info info;
		info.image = floor;
		info.frame = frame + get_variant(pos) * 16;
		info.size = mark::map::tile_size;
		info.pos = map_to_world(pos) - mark::vector<double>(0.5, 0.5) * tile_size;
		context.sprites[-1].emplace_back(info);
	}
}

std::string mark::map::serialize_terrain_type(terrain_type t) {
	switch (t) {
	case terrain_type::null:
		return "null";
	case terrain_type::abyss:
		return "abyss";
	case terrain_type::floor_1:
		return "floor_1";
	case terrain_type::floor_2:
		return "floor_2";
	case terrain_type::floor_3:
		return "floor_3";
	case terrain_type::wall:
		return "wall";
	default:
		throw std::bad_cast();
	}
}

mark::map::terrain_type mark::map::deserialize_terrain_type(const std::string& str) {
	if (str == "null") {
		return map::terrain_type::null;
	} else if (str == "abyss") {
		return map::terrain_type::abyss;
	} else if (str == "floor_1") {
		return map::terrain_type::floor_1;
	} else if (str == "floor_2") {
		return map::terrain_type::floor_2;
	} else if (str == "floor_3") {
		return map::terrain_type::floor_3;
	} else if (str == "wall") {
		return map::terrain_type::wall;
	} else {
		throw std::bad_cast();
	}
}

mark::map::map(mark::resource::manager& rm, mark::vector<size_t> size):
	m_rm(rm),
	m_size(size),
	m_terrain(size.x * size.y),
	m_variant(size.x * size.y),
	m_traversable(size.x * size.y) {
	for (auto& variant : m_variant) {
		variant = rm.random<unsigned>(0, 2);
	}
}

auto mark::map::size() const noexcept -> const mark::vector<size_t>& {
	return m_size;
}

auto mark::map::get(mark::vector<int> pos) const noexcept -> terrain_type {
	if (pos.x >= 0 && pos.x < m_size.x && pos.y >= 0 && pos.y < m_size.y) {
		return m_terrain[pos.x + m_size.x * pos.y];
	} else {
		return terrain_type::null;
	}
}

auto mark::map::get_variant(mark::vector<int> pos) const noexcept -> unsigned {
	if (pos.x >= 0 && pos.x < m_size.x && pos.y >= 0 && pos.y < m_size.y) {
		return m_variant[pos.x + m_size.x * pos.y];
	} else {
		return 0;
	}
}

void mark::map::set(mark::vector<int> pos, terrain_type type) noexcept {
	if (pos.x >= 0 && pos.x < m_size.x && pos.y >= 0 && pos.y < m_size.y) {
		m_terrain[pos.x + m_size.x * pos.y] = type;
	}
}

static auto make_directions()
{
	std::array<mark::vector<int>, 8> out;
	int j = 0;
	for (const auto i : mark::range(9)) {
		if (i != 4) {
			out[j] = mark::vector<int>(i % 3 - 1, i / 3 - 1);
			++j;
		}
	}
	return out;
}

const auto directions = make_directions();

auto nearest_traversable(
	const mark::map& map, const mark::vector<int>& pos, const size_t radius)
	-> mark::vector<int>
{
	if (!map.traversable(pos, radius)) {
		for (const auto i : mark::range(100)) {
			for (const auto neighbour : directions) {
				const auto cur = neighbour * i + pos;
				if (map.traversable(cur, radius)) {
					return cur;
				}
			}
		}
	}
	return pos;
}

static auto straight_exists(
	const mark::map& map,
	const mark::vector<double>& start,
	const mark::vector<double>& end,
	const double radius)
{
	const auto diff = end - start;
	const auto dir = mark::normalize(diff);
	const auto dist = mark::length(diff);
	for (double i = 0.0; i <= dist; i += mark::map::tile_size / 2.f) {
		if (!map.traversable(dir * i + start, radius)) {
			return false;
		}
	}
	return true;
}

static bool has_one(
	const std::vector<std::unique_ptr<Node>>& nodes,
	mark::vector<int> pos)
{
	return nodes.end() != std::find_if(
		nodes.begin(), nodes.end(),
		[&pos](const auto& node) {
		return node->pos == pos;
	});
}

static auto find_one(
	std::vector<Node>& nodes, const mark::vector<int>& pos) -> Node*
{
	auto node_it = std::find_if(
		nodes.begin(), nodes.end(), [&pos](const auto& node) {
			return pos == node.pos;
	});
	return node_it == nodes.end()
		? nullptr
		: &(*node_it);
}

static auto make_path(
	const mark::map& map, const Node* node,
	std::vector<mark::vector<double>> out = { })
	-> std::vector<mark::vector<double>>
{
	if (node) {
		out.push_back(map.map_to_world(node->pos));
		return make_path(map, node->parent, std::move(out));
	}
	return out;
}

// Jump Point Search Functions - Beginning

static auto is_diagonal(const mark::vector<int>& direction)
{
	return direction.x != 0 && direction.y != 0;
}

static auto turn_left(const mark::vector<int>& direction)
	-> mark::vector<int>
{
	return { -direction.y, direction.x };
}

static auto turn_right(const mark::vector<int>& direction)
	-> mark::vector<int>
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
		const auto p1 = !map.traversable(next + turn_left(direction), radius);
		const auto p2 = map.traversable(next + turn_left(direction) + direction, radius);
		const auto p3 = !map.traversable(next + turn_right(direction), radius);
		const auto p4 = map.traversable(next + turn_right(direction) + direction, radius);
	}
	return
		!is_diagonal(direction)
		&& (!map.traversable(next + turn_left(direction), radius)
			&& map.traversable(next + turn_left(direction) + direction, radius)
			|| !map.traversable(next + turn_right(direction), radius)
			&& map.traversable(next + turn_right(direction) + direction, radius));
}

static auto make_direction(const Node& node) -> mark::vector<int>
{
	if (node.parent) {
		auto direction = node.pos - node.parent->pos;
		direction.x = mark::sgn(direction.x);
		direction.y = mark::sgn(direction.y);
		return direction;
	} else {
		return { 0, 0 };
	}
}

static auto pruned_neighbours(
	const mark::map& map, const Node& node, const int radius)
	-> std::vector<mark::vector<int>>
{
	std::vector<mark::vector<int>> out;
	const auto direction = make_direction(node);
	if (direction.x == 0 && direction.y == 0) {
		for (const auto& direction : directions) {
			if (map.traversable(direction + node.pos, radius)) {
				out.push_back(direction + node.pos);
			}
		}
	} else if (is_diagonal(direction)) {
		const auto right = node.pos + mark::vector<int>(direction.x, 0);
		if (map.traversable(right, radius)) {
			out.push_back(right);
		}
		const auto top = node.pos + mark::vector<int>(0, direction.y);
		if (map.traversable(top, radius)) {
			out.push_back(top);
		}
		const auto edge = node.pos + direction;
		if (map.traversable(edge, radius)) {
			out.push_back(edge);
		}
	} else if (map.traversable(node.pos + direction, radius)) {
		out.push_back(node.pos + direction);
		if (!map.traversable(node.pos + turn_left(direction), radius)
			&& map.traversable(node.pos + turn_left(direction) + direction, radius)) {
			out.push_back(node.pos + turn_left(direction) + direction);
		}
		if (!map.traversable(node.pos + turn_right(direction), radius)
			&& map.traversable(node.pos + turn_right(direction) + direction, radius)) {
			out.push_back(node.pos + turn_right(direction) + direction);
		}
	}
	return out;
}

static auto jump(
	const mark::map& map,
	const mark::vector<int>& cur,
	const mark::vector<int>& direction,
	const mark::vector<int>& end,
	const int radius) -> std::optional<mark::vector<int>>
{
	const auto next = cur + direction;
	if (!map.traversable(next, radius)) {
		return { };
	} else if (next == end) {
		return next;
	} else if (has_forced_neighbours(map, next, direction, radius)) {
		return next;
	} else if (is_diagonal(direction)) {
		if (jump(map, next, { direction.x, 0 }, end, radius)
			|| jump(map, next, { 0, direction.y }, end, radius)) {
			return next;
		}
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
	for (const auto& neighbour : pruned_neighbours(map, cur, radius)) {
		const auto pos = jump(map, cur.pos, neighbour - cur.pos, end, radius);
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
	const mark::map& map, const mark::vector<int> &end, const int radius,
	std::vector<Node> open,
	std::vector<std::unique_ptr<Node>> closed)
	-> std::vector<mark::vector<double>>
{
	if (open.empty()) {
		return { };
	}
	const auto open_min_it = std::min_element(
		open.begin(), open.end(), [](const auto& n1, const auto& n2) {
		return n1.f < n2.f;
	});

	closed.push_back(std::make_unique<Node>(*open_min_it));
	auto& current = *closed.back();
	open.erase(open_min_it);

	if (current.pos == end) {
		return make_path(map, &current);
	}

	const auto successors = identify_successors(map, current, end, radius);
	open = std::accumulate(
		successors.begin(), successors.end(), std::move(open),
		[&](auto& open, const auto& successor) {
		auto neighbour_pos = successor.pos;
		if (has_one(closed, neighbour_pos)) {
			return open;
		}
		const auto neighbour = find_one(open, neighbour_pos);
		if (!neighbour) {
			open.push_back(successor);
		} else if (neighbour->f > successor.f) {
			neighbour->f = successor.f;
			neighbour->parent = successor.parent;
		}
		return open;
	});
	return find_path(map, end, radius, std::move(open), std::move(closed));
}

auto mark::map::find_path(
	const vector<double>& world_start,
	const vector<double>& world_end,
	const double world_radius) const
	-> std::vector<vector<double>>
{
	if (straight_exists(*this, world_start, world_end, world_radius)) {
		return { world_end };
	}
	const auto radius = static_cast<int>(std::ceil(world_radius / mark::map::tile_size));
	const auto end = ::nearest_traversable(*this, world_to_map(world_end), radius);
	const auto start = world_to_map(world_start);
	const auto f = static_cast<int>(mark::length(end - start));
	m_find_count++;
	return ::find_path(*this, end, radius, { Node{ start, f, nullptr } }, { });
}

auto mark::map::can_find() const -> bool
{ return m_find_count <= 5; }

auto mark::map::collide(mark::segment_t segment_) const -> std::optional<mark::vector<double>> {
	const auto direction = mark::normalize(segment_.second - segment_.first);
	constexpr const auto a = mark::map::tile_size / 2.0;
	// floating point error margin for comparing segments
	constexpr const auto margin = 8.;
	const mark::segment_t segment {
		segment_.first - direction * margin,
		segment_.second + direction * margin };
	const auto length = mark::length(segment.second - segment.first);
	auto prev = this->get(this->world_to_map(segment.first));
	for (double i = a; i < length + a; i += a) {
		const auto cur_pos = this->world_to_map(segment.first + i * direction);
		const auto cur = this->get(cur_pos);
		if (prev != cur) {
			const auto center = this->map_to_world(cur_pos);
			const std::array<segment_t, 4> borders {
				mark::segment_t{ { -a, -a - margin }, { -a, a + margin } },
				{ { -a - margin, a }, { a + margin, a } },
				{ { a, a + margin }, { a, -a - margin } },
				{ { a + margin, -a }, { -a - margin, -a } }
			};
			auto min = segment.second;
			double min_len = INFINITY;
			for (const auto& border : borders) {
				const auto intersection = intersect(segment, {
					border.first + center,
					border.second + center
				});
				if (!isnan(intersection.x)) {
					const auto len = mark::length(intersection - segment.first);
					if (len < min_len) {
						min = intersection;
						min_len = len;
					}
				}
			}
			return min;
		}
		prev = cur;
	}
	return { };
}

void mark::map::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << "map";
	out << Key << "size" << Value << BeginMap;
	out << Key << "x" << Value << m_size.x;
	out << Key << "y" << Value << m_size.y;
	out << EndMap;
	out << Key << "data" << Value << BeginSeq;
	for (const auto terrain : m_terrain) {
		out << serialize_terrain_type(terrain);
	}
	out << EndSeq;
	out << EndMap;
}

mark::map::map(
	mark::resource::manager& resource_manager,
	const YAML::Node& node) :
	map(resource_manager, node["size"].as<mark::vector<size_t>>()) {
	if (node["type"].as<std::string>() != "map") {
		std::runtime_error("BAD_DESERIALIZE");
	}
	m_terrain = { m_size.x * m_size.y, terrain_type::null };
	size_t i = 0;
	for (const auto cell : node["data"]) {
		m_terrain[i] = deserialize_terrain_type(cell.as<std::string>());
		i++;
	}
	this->calculate_traversable();
}

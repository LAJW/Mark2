#include "stdafx.h"
#include "algorithm.h"
#include "map.h"
#include "resource_manager.h"
#include "vector.h"
#include "sprite.h"
#include "tick_context.h"
#include "exception.h"
#include "base64.h"

// Make specific types of maps

using Node = mark::map::Node;

mark::map mark::map::make_cavern(mark::resource::manager& resource_manager)
{
	map map(resource_manager, { 1000, 1000 });
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<> dist_1_100(1, 10);
	std::uniform_int_distribution<> dist_0_3(0, 3);

	auto point = vector<int>(500, 500);
	for (int i = 0; i < 100; i++) {
		let direction = vector<int>(rotate(vector<float>(1, 0), dist_0_3(gen) * 90.f));
		let orto = vector<int>(rotate(vector<float>(direction), 90.f));
		let length = dist_1_100(gen);

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

mark::map mark::map::make_square(resource::manager& resource_manager)
{
	mark::map map(resource_manager, { 20, 20 });
	for (int x = 1; x < 20 - 1; x++) {
		for (int y = 1; y < 20 - 1; y++) {
			map.set({ x, y }, terrain_type::floor_1);
		}
	}
	map.calculate_traversable();
	return map;
}

// Map functionality

auto mark::map::world_to_map(const vector<double>& pos) const noexcept ->
	vector<int>
{
	return round(pos / map::tile_size)
		+ vector<int>(this->size() / size_t(2));
}

void mark::map::calculate_traversable()
{
	for (let pos : range(m_size)) {
		auto& arr = m_terrain[pos.x + pos.y * m_size.x].traversable;
		bool traversable = true;
		for (let radius : range<size_t>(0, 20)) {
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

auto mark::map::map_to_world(const vector<int>& pos) const noexcept ->
	vector<double>
{
	let center = vector<int>(this->size() / size_t(2));
	return vector<double>(pos - center) * map::tile_size;
}

auto mark::map::traversable(
	const vector<double>& pos,
	const double radius_) const -> bool
{
	let radius = size_t(std::ceil(radius_ / map::tile_size));
	return this->traversable(this->world_to_map(pos), radius);
}

auto mark::map::traversable(
	const vector<int>& pos,
	const size_t radius) const -> bool
{
	return pos.x >= 0 && pos.x < m_size.x && pos.y >= 0 && pos.y < m_size.y
		&& radius < 20 && m_terrain[pos.x + m_size.x * pos.y].traversable[radius];
}

auto mark::map::p_traversable(
	const vector<int>& i_pos,
	const size_t uradius) const -> bool
{
	if (uradius > 1) {
		let radius = static_cast<int>(uradius);
		let offset = vector<int>(radius, radius);
		for (let i : range(-offset, offset)) {
			if (length(i) <= radius) {
				let tile = this->get(i_pos + i);
				if (tile == terrain_type::null
					|| tile == terrain_type::wall) {
					return false;
				}
			}
		}
	} else {
		let tile = this->get(i_pos);
		if (tile == terrain_type::null
			|| tile == terrain_type::wall) {
			return false;
		}
	}
	return true;
}

void mark::map::tick(
	vector<double> world_tl,
	vector<double> world_br,
	tick_context& context)
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
	let floor = m_rm.get().image("jungle-1.png");
	transform(
		range.begin(),
		range.end(),
		back_inserter(context.sprites[-1]),
		[&] (let pos) {
		let frame = [&] {
			let ctl = this->get(pos - vector<int>(1, 1));
			let cbl = this->get(pos - vector<int>(1, 0));
			let ctr = this->get(pos - vector<int>(0, 1));
			let cbr = this->get(pos);
			return ((ctl == terrain_type::floor_1) & 1)
				| ((cbl == terrain_type::floor_1) & 1) << 1
				| ((ctr == terrain_type::floor_1) & 1) << 2
				| ((cbr == terrain_type::floor_1) & 1) << 3;

		}();
		sprite info;
		info.image = floor;
		info.frame = frame + get_variant(pos) * 16;
		info.size = map::tile_size;
		info.pos = map_to_world(pos) - vector<double>(0.5, 0.5) * tile_size;
		return info;
	});
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

auto mark::map::deserialize_terrain_type(const std::string& str)
	-> map::terrain_type
{
	if (str == "null") {
		return terrain_type::null;
	} else if (str == "abyss") {
		return terrain_type::abyss;
	} else if (str == "floor_1") {
		return terrain_type::floor_1;
	} else if (str == "floor_2") {
		return terrain_type::floor_2;
	} else if (str == "floor_3") {
		return terrain_type::floor_3;
	} else if (str == "wall") {
		return terrain_type::wall;
	} else {
		throw std::bad_cast();
	}
}

mark::map::map(resource::manager& rm, const vector<size_t>& size)
	: m_rm(rm)
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

auto mark::map::get(const vector<int>& pos) const noexcept
	-> terrain_type
{
	if (pos.x >= 0 && pos.x < m_size.x && pos.y >= 0 && pos.y < m_size.y) {
		return m_terrain[pos.x + m_size.x * pos.y].type;
	} else {
		return terrain_type::null;
	}
}

auto mark::map::get_variant(const vector<int>& pos) const noexcept -> unsigned
{
	if (pos.x >= 0 && pos.x < m_size.x && pos.y >= 0 && pos.y < m_size.y) {
		return m_terrain[pos.x + m_size.x * pos.y].variant;
	} else {
		return 0;
	}
}

void mark::map::set(
	const vector<int>& pos, terrain_type type) noexcept
{
	if (pos.x >= 0 && pos.x < m_size.x && pos.y >= 0 && pos.y < m_size.y) {
		m_terrain[pos.x + m_size.x * pos.y].type = type;
	}
}

// 16-point star
std::array<mark::vector<int>, 16> directions{ mark::vector<int>
	{ -2, -1 }, { -1, -1 }, { -1, -2 },
	{ 2, -1 }, { 1, -1 }, { 1, -2 },
	{ -2, 1 }, { -1, 1 }, { -1, 2 },
	{ 2, 1 }, { 1, 1 }, { 1, 2 },
	{ 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 },
};

auto nearest_traversable(
	const mark::map& map,
	const mark::vector<int>& end,
	const size_t radius)
	-> mark::vector<int>
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
	const size_t radius)
	-> mark::vector<int>
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

static bool has_one(
	const std::vector<std::unique_ptr<Node>>& nodes,
	mark::vector<int> pos)
{
	return any_of(
		nodes.begin(), nodes.end(),
		[&pos](let& node) { return node->pos == pos; });
}

static auto find_one(
	std::vector<Node>& nodes, const mark::vector<int>& pos) -> Node*
{
	auto node_it = find_if(
		nodes.begin(), nodes.end(), [&pos](let& node) {
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
		return make_path(map, node->parent, move(out));
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
		let p1 = !map.traversable(next + turn_left(direction), radius);
		let p2 = map.traversable(next + turn_left(direction) + direction, radius);
		let p3 = !map.traversable(next + turn_right(direction), radius);
		let p4 = map.traversable(next + turn_right(direction) + direction, radius);
	}
	return
		!is_diagonal(direction)
		&& ((!map.traversable(next + turn_left(direction), radius)
			&& map.traversable(next + turn_left(direction) + direction, radius))
			|| (!map.traversable(next + turn_right(direction), radius)
			&& map.traversable(next + turn_right(direction) + direction, radius)));
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
	let direction = make_direction(node);
	if (direction.x == 0 && direction.y == 0) {
		for (let& next_direction : directions) {
			if (map.traversable(next_direction + node.pos, radius)) {
				out.push_back(next_direction + node.pos);
			}
		}
	} else if (is_diagonal(direction)) {
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
	let next = cur + direction;
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
	const mark::map& map, const mark::vector<int> &end, const int radius,
	std::vector<Node> open,
	std::vector<std::unique_ptr<Node>> closed)
	-> std::vector<mark::vector<double>>
{
	if (open.empty()) {
		return { };
	}
	let open_min_it = min_element(
		open.begin(), open.end(), [](let& n1, let& n2) {
		return n1.f < n2.f;
	});

	closed.push_back(std::make_unique<Node>(*open_min_it));
	auto& current = *closed.back();
	open.erase(open_min_it);

	if (current.pos == end) {
		return make_path(map, &current);
	}

	let successors = identify_successors(map, current, end, radius);
	open = accumulate(
		successors.begin(), successors.end(), move(open),
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
	const double world_radius) const
	-> std::vector<vector<double>>
{
	if (straight_exists(*this, world_start, world_end, world_radius)) {
		return { world_end };
	}
	let radius = static_cast<int>(std::ceil(world_radius / map::tile_size));
	let start = world_to_map(world_start);
	// Start not traversable - get as quickly to the world as possible
	if (!this->traversable(start, radius)) {
		let end = ::nearest_traversable(
			*this, start, radius);
		return { this->map_to_world(end) };
	} else {
		let end = ::nearest_traversable(
			*this, start, world_to_map(world_end), radius);
		let f = static_cast<int>(length(end - start));
		m_find_count++;
		return ::find_path(*this, end, radius, { Node{ start, f, nullptr } }, { });
	}
}

auto mark::map::can_find() const -> bool
{ return m_find_count <= 5; }

const std::array<mark::segment_t, 4> square { mark::segment_t
	{ { -1, -1 }, { -1, 1 } },
	{ { -1, 1 }, { 1, 1 } },
	{ { 1, 1 }, { 1, -1 } },
	{ { 1, -1 }, { -1, -1 } }
};

auto mark::map::collide(const segment_t& segment) const
	-> std::optional<vector<double>>
{
	let direction = normalize(segment.second - segment.first);
	constexpr let a = map::tile_size / 2.0;
	// floating point error margin for comparing segments
	let length = mark::length(segment.second - segment.first);
	std::optional<vector<double>> min;
	for (double i = a; i < length + a; i += a) {
		let cur_pos = this->world_to_map(segment.first + i * direction);
		let cur = this->get(cur_pos);
		if (cur != terrain_type::floor_1) {
			let center = this->map_to_world(cur_pos);
			double min_len = INFINITY;
			for (let& border : square) {
				let intersection = intersect(segment, {
					border.first * a + center,
					border.second * a + center
				});
				if (intersection) {
					let len = mark::length(*intersection - segment.first);
					if (len < min_len) {
						min = *intersection;
						min_len = len;
					}
				}
			}
		}
		if (min) {
			return min;
		}
	}
	return { };
}

void mark::map::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << "map";
	out << Key << "size" << Value << BeginMap;
	out << Key << "x" << Value << m_size.x;
	out << Key << "y" << Value << m_size.y;
	out << EndMap;
	std::string data;
	for (let terrain : m_terrain) {
		data.push_back(static_cast<unsigned char>(terrain.type));
	}
	out << Key << "data" << Value << base64_encode(data);
	out << EndMap;
}

mark::map::map(
	resource::manager& resource_manager,
	const YAML::Node& node)
	: map(resource_manager, node["size"].as<vector<size_t>>())
{
	if (node["type"].as<std::string>() != "map") {
		std::runtime_error("BAD_DESERIALIZE");
	}
	m_terrain = { m_size.x * m_size.y, terrain() };
	size_t i = 0;
	auto data = base64_decode(node["data"].as<std::string>());
	for (let ch : data) {
		m_terrain[i].type = static_cast<terrain_type>(ch);
		m_terrain[i].variant = resource_manager.random(0, 2);
		i++;
	}
	this->calculate_traversable();
}

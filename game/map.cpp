#include "stdafx.h"
#include <algorithm>
#include <random>
#include "algorithm.h"
#include "map.h"
#include "resource_manager.h"
#include "vector.h"
#include "sprite.h"
#include "tick_context.h"
#include "exception.h"

// Make specific types of maps

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
	return map;
}

mark::map mark::map::make_square(mark::resource::manager& resource_manager) {
	mark::map map(resource_manager, { 20, 20 });
	for (int x = 1; x < 20 - 1; x++) {
		for (int y = 1; y < 20 - 1; y++) {
			map.set({ x, y }, mark::map::terrain_type::floor_1);
		}
	}
	return map;
}

// Map functionality

auto mark::map::world_to_map(mark::vector<double> pos) const noexcept ->
	mark::vector<int> {
	return mark::round(pos / mark::map::tile_size)
		+ mark::vector<int>(this->size() / size_t(2));
}

auto mark::map::map_to_world(mark::vector<int> pos) const noexcept ->
	mark::vector<double> {
	const auto center = mark::vector<int>(this->size() / size_t(2));
	return mark::vector<double>(pos - center) * mark::map::tile_size;
}

auto mark::map::traversable(
	const mark::vector<double> pos,
	const double radius_) const -> bool {
	const auto radius = size_t(std::ceil(radius_ / mark::map::tile_size));
	return this->traversable(this->world_to_map(pos), radius);
}

auto mark::map::traversable(
	const mark::vector<int> i_pos,
	const size_t uradius) const -> bool {
	if (uradius > 1) {
		const auto radius = static_cast<int>(uradius);
		const auto offset = mark::vector<int>(radius, radius);
		for (const auto i : mark::enumerate(-offset, offset)) {
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
	for (const auto pos : mark::enumerate(tl, br)) {
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
	m_variant(size.x * size.y) {
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

struct Node {
	mark::vector<int> pos;
	int f = 0; // distance from starting + distance from ending (h)
	Node* parent = nullptr;
};

auto mark::map::find_path(
	const mark::vector<double> world_start,
	mark::vector<double> world_end,
	const double radius) const -> std::vector<mark::vector<double>> {
	const auto map_radius = static_cast<int>(std::ceil(radius / mark::map::tile_size));
	// if end is not traversable, find nearest traversable point, and update world end
	if (!this->traversable(world_end, radius)) {
		for (int r = 0; r < 100; r++) {
			for (int d = 0; d < 8; d++) {
				const auto direction = mark::rotate(
					mark::vector<double>(r * mark::map::tile_size, 0.0),
					static_cast<double>(d * 45));
				if (traversable(world_end + direction, radius)) {
					world_end = world_end + direction;
					goto end;
				}
			}
		}
	end:;
	}
	// check if path is reachable directly
	bool straight_exists = true;
	const auto dir = mark::normalize(world_end - world_start);
	const auto dist = mark::length(world_end - world_start);
	for (double i = 0.0; i <= dist; i += tile_size / 2.f) {
		if (!this->traversable(dir * i + world_start, radius)) {
			straight_exists = false;
			break;
		}
	}
	if (straight_exists) {
		return { world_end };
	}
	const auto start = world_to_map(world_start);
	const auto end = world_to_map(world_end);
	std::vector<Node> open = { Node{ start, static_cast<int>(mark::length(end - start)), nullptr } };
	std::vector<std::unique_ptr<Node>> closed;

	m_find_count++;
	while (!open.empty()) {
		auto min_it = open.begin();
		for (auto it = open.begin(), end = open.end(); it != end; it++) {
			if (min_it->f > it->f) {
				min_it = it;
			}
		}
		closed.push_back(std::make_unique<Node>(*min_it));
		auto& current = closed.back();
		open.erase(min_it);

		if (current->pos == end) {
			std::vector<mark::vector<double>> out;
			auto node = current.get();
			while (node) {
				out.push_back(map_to_world(node->pos));
				node = node->parent;
			}
			return out;
		}

		for (int i = 0; i < 9; i++) {
			if (i == 4) {
				continue;
			}
			auto neighbour_pos = current->pos + mark::vector<int>{ i % 3 - 1, i / 3 - 1 };
			const auto traversable = this->traversable(neighbour_pos, map_radius);
			const auto isClosed = closed.end() != std::find_if(closed.begin(), closed.end(), [&neighbour_pos](std::unique_ptr<Node>& node) {
				return node->pos == neighbour_pos;
			});
			if (!traversable || isClosed) {
				continue;
			}
			auto neighbour_it = std::find_if(open.begin(), open.end(), [&neighbour_pos](const Node& node) {
				return neighbour_pos == node.pos;
			});
			const auto f = current->f + (i % 2 ? 10 : 14);
			if (neighbour_it == open.end()) {
				open.push_back({ neighbour_pos, f, current.get() });
			} else if (neighbour_it->f > f) {
				neighbour_it->f = f;
				neighbour_it->parent = current.get();
			}
		}
	}
	return std::vector<mark::vector<double>>();
}

auto mark::map::can_find() const -> bool {
	return m_find_count <= 5;
}

auto mark::map::collide(mark::segment_t segment_) const -> mark::vector<double> {
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
	return { NAN, NAN };
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
}

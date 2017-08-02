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
	const auto radius = static_cast<int>(uradius);
	const auto size = this->size();
	const auto offset = mark::vector<int>(radius, radius);
	for (const auto i : mark::enumerate(offset * 2)) {
		if (mark::length(offset - i) <= radius) {
			const auto tile = this->at(i_pos + i - offset);
			if (!tile || *tile == terrain_type::null
				|| *tile == terrain_type::wall) {
				return false;
			}
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
	for (const auto i : mark::enumerate(br - tl)) {
		const auto pos = i + tl;
		const auto point = this->at(pos);
		if (point && *point == terrain_type::floor_1) {
			mark::sprite::info info;
			info.image = m_rm.get().image("floor.png");
			info.size = mark::map::tile_size;
			info.pos = map_to_world(pos);
			context.sprites[-1].emplace_back(info);
		}
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

mark::map::terrain_type mark::map::deserialize(const std::string& str) {
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
	m_terrain(size.x * size.y) { }

auto mark::map::size() const noexcept -> const mark::vector<size_t>& {
	return m_size;
}

auto mark::map::at(mark::vector<int> pos) noexcept -> terrain_type* {
	const map& self = *this;
	return const_cast<terrain_type*>(self.at(pos));
}

auto mark::map::at(mark::vector<int> pos) const noexcept ->
	const terrain_type* {
	if (pos.x >= 0 && pos.x < m_size.x && pos.y >= 0 && pos.y < m_size.y) {
		return &m_terrain[pos.x + m_size.x * pos.y];
	} else {
		return nullptr;
	}
}

struct Node {
	mark::vector<int> pos;
	int f = 0; // distance from starting + distance from ending (h)
	Node* parent = nullptr;
};

auto mark::map::find_path(
	mark::vector<double> world_start,
	mark::vector<double> world_end, double radius) const ->
	std::vector<mark::vector<double>> {
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
	for (double i = 0.0; i < dist; i += 24.0) {
		const auto cur = dir * i + world_start;
		if (!this->traversable(cur, radius)) {
			straight_exists = false;
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

auto mark::map::collide(mark::segment_t segment) const -> mark::vector<double> {
	const auto length = mark::length(segment.second - segment.first);
	const auto direction = mark::normalize(segment.second - segment.first);
	// previous block - to skip over already checked blocks
	auto prev = mark::vector<double>(NAN, NAN);
	for (double i = 0; i < length; i+= mark::map::tile_size / 2.0) {
		const auto cur = segment.first + i * direction;
		if (cur == prev) {
			continue;
		}
		const auto pos = this->world_to_map(cur);
		if (this->at(pos)) {
			// do something
		}
		prev = cur;
	}
	return { NAN, NAN };
}

void mark::map::serialize(YAML::Emitter& out) const {
}

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
				if (const auto tile = map.at(point + direction * j + orto * k)) {
					*tile = mark::map::terrain_type::floor_1;
				}
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
			*map.at({ x, y }) = mark::map::terrain_type::floor_1;
		}
	}
	return map;
}

mark::map::map(
	mark::resource::manager& resource_manager,
	const YAML::Node& node):
	m_rm(resource_manager),
	m_size({ 1000, 1000 }),
	m_terrain(1000 * 1000) {
	throw std::runtime_error("NOT IMPLEMENTED");
}

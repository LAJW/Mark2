#include <random>
#include <algorithm>
#include "map.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "vector.h"
#include "sprite.h"
#include "terrain_floor.h"
#include "terrain_wall.h"
#include "tick_context.h"

static auto world_to_map(
	const mark::vector<double>& pos,
	const mark::vector<int>& map_size) {
	return mark::vector<int>(static_cast<int>(std::round(pos.x / mark::terrain::grid_size)),
			static_cast<int>(std::round(pos.y / mark::terrain::grid_size)))
		+ map_size / 2;
}

static auto map_to_world(const mark::vector<int>& pos, const mark::vector<int>& map_size) {
	return mark::vector<double>(pos - map_size / 2) * mark::terrain::grid_size;
}

static auto fix_corners(
	mark::resource::manager& resource_manager,
	std::vector<std::vector<std::shared_ptr<mark::terrain::base>>> floor) {
	for (size_t x = 1, size_x = floor.size(); x < size_x - 1; x++) {
		for (size_t y = 1, size_y = floor[0].size(); y < size_y - 1; y++) {
			auto cur = dynamic_cast<mark::terrain::floor*>(floor[x][y].get());
			auto tl = dynamic_cast<mark::terrain::wall*>(floor[x - 1][y - 1].get());
			auto t = dynamic_cast<mark::terrain::wall*>(floor[x][y - 1].get());
			auto tr = dynamic_cast<mark::terrain::wall*>(floor[x + 1][y - 1].get());
			auto l = dynamic_cast<mark::terrain::wall*>(floor[x - 1][y].get());
			auto r = dynamic_cast<mark::terrain::wall*>(floor[x + 1][y].get());
			auto bl = dynamic_cast<mark::terrain::wall*>(floor[x - 1][y + 1].get());
			auto b = dynamic_cast<mark::terrain::wall*>(floor[x][y + 1].get());
			auto br = dynamic_cast<mark::terrain::wall*>(floor[x + 1][y + 1].get());
			auto rand = resource_manager.random(0, 2) * 13;
			auto rand2 = resource_manager.random(0.0, 1.0);
			if (cur) {
				if (t) {
					if (l) {
						cur->variant(0 + rand);
					} else if (r) {
						cur->variant(2 + rand);
					} else {
						cur->variant(1 + rand);
					}
				} else if (b) {
					if (l) {
						cur->variant(6 + rand);
					} else if (r) {
						cur->variant(8 + rand);
					} else {
						cur->variant(7 + rand);
					}
				} else if (l) {
					cur->variant(3 + rand);
				} else if (r) {
					cur->variant(5 + rand);
				} else if (tl) {
					cur->variant(12 + rand);
				} else if (tr) {
					cur->variant(11 + rand);
				} else if (bl) {
					cur->variant(10 + rand);
				} else if (br) {
					cur->variant(9 + rand);
				} else {
					int rand3;
					if (rand2 > 0.05) {
						rand3 = 0;
					} else if (rand2 > 0.025) {
						rand3 = 1;
					} else {
						rand3 = 2;
					}
					cur->variant(4 + rand3 * 13);
				}
			}
		}
	}
	return floor;
}

static auto create_walls(mark::resource::manager& resource_manager, std::vector<std::vector<std::shared_ptr<mark::terrain::base>>> floor) {
	for (size_t x = 1, size_x = floor.size(); x < size_x - 1; x++) {
		for (size_t y = 1, size_y = floor[0].size(); y < size_y - 1; y++) {
			if (std::dynamic_pointer_cast<mark::terrain::floor>(floor[x][y])) {
				for (size_t i = 0; i < 9; i++) {
					mark::vector<size_t> cur(x + i % 3 - 1, y + i / 3 - 1);
					auto& neighbour = floor[cur.x][cur.y];
					if (!neighbour) {
						neighbour = std::make_shared<mark::terrain::wall>(resource_manager);
					}
				}
			}
		}
	}
	return floor;
}

static auto make_map(mark::resource::manager& resource_manager) {
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<> dist_1_100(1, 10);
	std::uniform_int_distribution<> dist_0_3(0, 3);

	std::vector<std::vector<std::shared_ptr<mark::terrain::base>>> floor(1000, std::vector<std::shared_ptr<mark::terrain::base>>(1000, nullptr));
	auto point = mark::vector<int>(500, 500);
	for (int i = 0; i < 100; i++) {
		const auto direction = mark::vector<int>(mark::rotate(mark::vector<float>(1, 0), dist_0_3(gen) * 90.f));
		const auto orto = mark::vector<int>(mark::rotate(mark::vector<float>(direction), 90.f));
		const auto length = dist_1_100(gen);

		for (int j = -5; j < length + 5; j++) {
			const auto step = point + direction * j;
			for (int k = -3; k <= 3; k++) {
				const auto cur = step + orto * k;
				if (cur.x > 0 && cur.x < 1000 && cur.y > 0 && cur.y < 1000) {
					floor[cur.x][cur.y] = std::make_shared<mark::terrain::floor>(resource_manager);
				}
			}
		}
		point += direction * length;
	}
	return fix_corners(resource_manager, create_walls(resource_manager, std::move(floor)));
}

auto mark::map::traversable(const mark::vector<double> pos, const double radius) const -> bool {
	return this->traversable(world_to_map(pos, this->size()), static_cast<int>(std::ceil(radius / mark::terrain::grid_size)));
}

auto mark::map::traversable(const mark::vector<int> i_pos, const int radius) const -> bool {
	const auto size = this->size();
	for (int x = -radius; x <= radius; x++) {
		for (int y = -radius; y <= radius; y++) {
			if (std::sqrt(x * x + y* y) <= radius) {
				const auto pos = i_pos + mark::vector<int>(x, y);
				if (!(
					pos.x >= 0 && pos.x < size.x
					&& pos.y >= 0 && pos.y < size.y
					&& m_terrain[pos.x][pos.y]
					&& m_terrain[pos.x][pos.y]->traversable()
				)) {
					return false;
				}
			}
		}
	}
	return true;
}

void mark::map::tick(mark::vector<double> world_tl, mark::vector<double> world_br, mark::tick_context& context) {
	m_find_count = 0;
	const auto size = this->size();
	const auto tl = world_to_map(world_tl, size);
	const auto br = world_to_map(world_br, size);
	std::vector<mark::sprite> sprites;
	for (int x = std::max(tl.x, 0); x < std::min(size.x, br.x); x++) {
		for (int y = std::max(tl.y, 0); y < std::min(size.y, br.y); y++) {
			const auto& point = m_terrain[x][y];
			if (point) {
				m_terrain[x][y]->tick(context, mark::vector<int>(x, y) - size / 2);
			}
		}
	}
}

auto mark::map::size() const->mark::vector<int> {
	return { static_cast<int>(m_terrain.size()), static_cast<int>(m_terrain[0].size()) };
}

struct Node {
	mark::vector<int> pos;
	int f = 0; // distance from starting + distance from ending (h)
	Node* parent = nullptr;
};

auto mark::map::find_path(mark::vector<double> world_start, mark::vector<double> world_end, double radius) const -> std::vector<mark::vector<double>> {
	const auto map_radius = static_cast<int>(std::ceil(radius / mark::terrain::grid_size));
	// if end is not traversable, find nearest traversable point, and update world end
	if (!this->traversable(world_end, radius)) {
		for (int r = 0; r < 100; r++) {
			for (int d = 0; d < 8; d++) {
				const auto direction = mark::rotate(mark::vector<double>(r * mark::terrain::grid_size, 0.0), static_cast<double>(d * 45));
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
	const auto size = this->size();
	const auto start = world_to_map(world_start, size);
	const auto end = world_to_map(world_end, size);
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
				out.push_back(map_to_world(node->pos, size));
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
	const auto size = this->size();
	const auto length = mark::length(segment.second - segment.first);
	const auto direction = mark::normalize(segment.second - segment.first);
	// previous block - to skip over already checked blocks
	auto prev = mark::vector<double>(NAN, NAN);
	for (double i = 0; i < length; i+= mark::terrain::grid_size / 2.0) {
		const auto cur = segment.first + i * direction;
		if (cur == prev) {
			continue;
		}
		const auto pos = world_to_map(cur, this->size());
		if (pos.x >= 0 && pos.x < size.x
			&& pos.y >= 0 && pos.y < size.y
			&& m_terrain[pos.x][pos.y]) {
			const auto intersection = m_terrain[pos.x][pos.y]->collide(
				map_to_world(pos, this->size()),
				segment
			);
			if (!std::isnan(intersection.x)) {
				return intersection;
			}
		}
		prev = cur;
	}
	return { NAN, NAN };
}

void mark::map::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << "map";
	out << Key << "terrain" << BeginSeq;
	for (unsigned x = 0, width = m_terrain.size(); x < width; x++) {
		const auto& row = m_terrain[x];
		for (unsigned y = 0, height = row.size(); y < height; y++) {
			const auto& tile = row[y];
			if (tile) {
				out << BeginMap;
				out << Key << "x" << Value << x;
				out << Key << "y" << Value << y;
				out << Key << "terrain" << Value;
				tile->serialize(out);
				out << EndMap;
			}
		}
	}
	out << EndSeq;
}

mark::map mark::map::make_cavern(mark::resource::manager& resource_manager) {
	return mark::map(make_map(resource_manager));
}

mark::map mark::map::make_square(mark::resource::manager& resource_manager) {
	mark::map::terrain_t terrain(20, std::vector<std::shared_ptr<mark::terrain::base>>(20, nullptr));
	for (size_t x = 1; x < 20 - 1; x++) {
		for (size_t y = 1; y < 20 - 1; y++) {
			auto& cell = terrain[x][y];
			cell = std::make_shared<mark::terrain::floor>(resource_manager);
		}
	}
	return fix_corners(resource_manager, create_walls(resource_manager, std::move(terrain)));
}

mark::map::map(mark::resource::manager& resource_manager, const YAML::Node& node) {
	std::vector<std::vector<std::shared_ptr<mark::terrain::base>>> floor(1000, std::vector<std::shared_ptr<mark::terrain::base>>(1000, nullptr));

	for (const auto& child : node["terrain"]) {
		const auto x = child["x"].as<unsigned>();
		const auto y = child["y"].as<unsigned>();
		floor[x][y] = mark::terrain::base::deserialize(resource_manager, child["terrain"]);
	}
	m_terrain = std::move(floor);
}

mark::map::map(mark::map::terrain_t terrain)
	:m_terrain(terrain) { }
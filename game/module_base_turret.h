#pragma once
#include "stdafx.h"
#include "module_base.h"
#include <deque>
#include <variant>

namespace mark {
namespace unit {
class base;
}
namespace module {
class base_turret : public module::base {
public:
	void target(vector<double> pos) override;
	void shoot(vector<double> pos, bool release) override;
	void queue(vector<double> pos, bool release) override;
	auto passive() const noexcept -> bool override;
protected:
	base_turret(
		vector<unsigned> size,
		const std::shared_ptr<const resource::image>& image);
	base_turret(resource::manager&, const YAML::Node& node);

	void tick_ai();
	// Returns true if there are objects still to be destroyed before
	// switching to manual control

	// DEPRECATE during this commit
	auto shoot() const -> bool;
	auto target() const -> std::optional<vector<double>>;
	void serialize_base(YAML::Emitter&) const;
private:
	using target_type = std::pair<bool, vector<double>>;
	using queue_type = std::deque<
		std::pair<
			std::weak_ptr<unit::base>,
			vector<double>>>;
	std::variant<target_type, queue_type> m_target;
};
} }
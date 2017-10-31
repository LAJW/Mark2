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
	auto passive() const noexcept -> bool override;
	void command(const command::any&) override final;
protected:
	base_turret(
		vector<unsigned> size,
		const std::shared_ptr<const resource::image>& image);
	base_turret(resource::manager&, const YAML::Node& node);

	void tick();
	auto shoot() const -> bool;
	auto target() const -> std::optional<vector<double>>;
	void serialise(YAML::Emitter&) const;
private:
	void target(vector<double> pos);
	void queue(vector<double> pos, bool release);

	using target_type = std::pair<bool, vector<double>>;
	using queue_type = std::deque<
		std::pair<
			std::weak_ptr<unit::base>,
			vector<double>>>;
	std::variant<target_type, queue_type> m_target;
};
} }
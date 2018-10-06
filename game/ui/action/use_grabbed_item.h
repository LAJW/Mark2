#pragma once
#include "base.h"

namespace mark {
namespace ui {
namespace action {

class use_grabbed_item final : public base
{
public:
	use_grabbed_item(
		vi32 drop_pos,
		const std::unordered_map<std::string, YAML::Node>& blueprints);
	void execute(const execute_info& info) override;

private:
	vi32 m_drop_pos;
	const std::unordered_map<std::string, YAML::Node>& m_blueprints;
};

} // namespace action
} // namespace ui
} // namespace mark

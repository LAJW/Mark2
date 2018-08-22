#pragma once
#include "node.h"
#include "window.h"
#include <stdafx.h>

namespace mark {

namespace ui {
class ui;
struct event;

class container final : public window
{
public:
	struct info final : window::info
	{
		optional<resource::manager&> rm;
		optional<module::cargo&> container;
		optional<mark::ui::ui&> ui;
	};
	container(const info& info);
	void update(update_context&) override;
	[[nodiscard]] handler_result click(const event& event) override;
	[[nodiscard]] const module::cargo& cargo() const;
	[[nodiscard]] vi32 size() const override;
	[[nodiscard]] vi32 pos() const noexcept override;

private:
	void attach(vi32 pos, interface::item&);
	resource::image_ptr m_cargo_bg;
	resource::image_ptr m_header;
	resource::image_ptr m_font;
	const ui& m_ui;
	module::cargo& m_container;
	std::vector<const mark::interface::item*> m_prev_contents;
};
} // namespace ui
} // namespace mark

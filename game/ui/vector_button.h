#pragma
#include <ui/button.h>

namespace mark {
namespace ui {

class vector_button final : public button_base
{
public:
	struct info : button_base::info
	{
		/// Text to be displayed on the button
		std::string title;
		resource::image_ptr font = nullptr;
	};
	vector_button(const info&);

	void update(update_context&) override;

private:
	void render(update_context&) const;

	const not_null<resource::image_ptr> m_font;
	const std::string m_title;
	double m_opacity = .0;
};

} // namespace ui
} // namespace mark

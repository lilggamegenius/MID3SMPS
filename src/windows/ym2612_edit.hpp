#pragma once

#include "main_window.hpp"

namespace MID3SMPS {
	class ym2612_edit : public window<ym2612_edit>{
	private:
		friend class main_window;
		void render_impl();
		void render_children_impl();
		void on_close_impl();
		[[nodiscard]] bool keep_impl() const;
	};
} // MID3SMPS
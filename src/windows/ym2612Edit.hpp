#pragma once

#include "MainWindow.hpp"

namespace MID3SMPS {
	class YM2612Edit : public Window<YM2612Edit>{
	private:
		friend class MainWindow;

		friend class Window;
		void render_impl();
		void render_children_impl();
		void on_close_impl();
		[[nodiscard]] bool keep_impl() const;

	};
} // MID3SMPS
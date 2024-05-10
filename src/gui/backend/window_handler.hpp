#pragma once

#include <atomic>
#include <memory>
#include <imguiwrap.h>

namespace MID3SMPS {
	class main_window;
}

struct fps_idling{
	uint_fast8_t fps_idle = 5;         // FPS when idling_
	bool  enable_idling = true;   // a bool to enable/disable idling_
	bool  is_idling = false;      // an output parameter filled by the runner
	std::atomic<uint_fast8_t> overrides = 0;
	bool override_this_frame = false; // Set whenever an override is requested and cleared every frame.

	[[nodiscard]] bool is_idle_override() const noexcept{
		return override_this_frame || overrides > 0;
	}

	class override{
		friend struct fps_idling;

		explicit override() noexcept;

	public:
		~override() noexcept;
	};

	[[nodiscard]] static override get_override(){
		return override();
	}
};

class window_handler {
	std::unique_ptr<MID3SMPS::main_window> mainWindow;
	ImGuiWrapConfig config_;
	ImFont* main_font = nullptr;
	ImFont* main_font_bold = nullptr;

public:
	fps_idling idling;

	[[nodiscard]] const ImGuiWrapConfig& config() const noexcept;

	window_handler();
	void main_loop_init();
	ImGuiWrapperReturnType main_loop_step();

	static void idle_by_sleeping();
	void reload_fonts(float pixel_size = 15.f);

private:
	[[nodiscard]] /*consteval*/ static ImFontConfig generate_font_config();
};

namespace MID3SMPS {
	extern window_handler handler;
	#ifdef DEBUG
	constexpr bool debug_mode = true;
	extern bool show_demo_window;
	#else
	constexpr bool debug_mode = false;
	#endif
}
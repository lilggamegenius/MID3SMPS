#pragma once

#include <atomic>
#include <list>
#include <memory>
#include <imguiwrap.h>

namespace MID3SMPS {
	class main_window;
}

struct FpsIdling{
	uint_fast8_t fpsIdle = 5;         // FPS when idling_
	bool  enableIdling = true;   // a bool to enable/disable idling_
	bool  isIdling = false;      // an output parameter filled by the runner
	std::atomic<uint_fast8_t> overrides = 0;

	[[nodiscard]] bool is_idle_override() const{
		return overrides > 0;
	}

	class override{
		friend struct FpsIdling;
		FpsIdling* ref = nullptr;

		explicit override(FpsIdling& ref_) : ref(&ref_) {
			++ref->overrides;
		}

	public:
		constexpr override() = default;

		constexpr override(override&& other) noexcept {
			std::swap(ref, other.ref);
		}
		override(const override &other) : override(*other.ref){}

		constexpr ~override() noexcept{
			if(!ref) { return; }
			--ref->overrides;
		}

		constexpr override& operator =(override &&other) noexcept{
			std::swap(ref, other.ref);
			return *this;
		}

		constexpr override& operator =(const override &other) noexcept{
			if(this == &other) { return *this; }
			ref = other.ref;
			++ref->overrides;
			return *this;
		}

		constexpr operator bool() const {
			return ref != nullptr;
		}
	};

	[[nodiscard]] override get_override(){
		return override(*this);
	}
};

class window_handler {
	std::unique_ptr<MID3SMPS::main_window> mainWindow;
	ImGuiWrapConfig config_;
	FpsIdling idling_;
	ImFont* main_font_ = nullptr;
	ImFont* main_font_bold_ = nullptr;

public:

	[[nodiscard]] const ImGuiWrapConfig& config() const noexcept;
	[[nodiscard]] FpsIdling& idling() noexcept;

	window_handler();
	void main_loop_init();
	ImGuiWrapperReturnType main_loop_step();

	void idle_by_sleeping();
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
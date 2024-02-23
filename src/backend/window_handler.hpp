#pragma once

#include <atomic>
#include <imguiwrap.h>
#include <list>
#include <memory>

namespace MID3SMPS {
	class main_window;
}

struct FpsIdling{
	uint_fast8_t fpsIdle = 5;         // FPS when idling_
	bool  enableIdling = true;   // a bool to enable/disable idling_
	bool  isIdling = false;      // an output parameter filled by the runner
	std::atomic<uint_fast8_t> overrides = 0;

	[[nodiscard]] bool isIdleOverride() const{
		return overrides > 0;
	}

	class Override{
		friend struct FpsIdling;
		FpsIdling* ref = nullptr;

		constexpr explicit Override(FpsIdling& ref_) : ref(&ref_) {
			++ref->overrides;
		}

	public:
		constexpr Override() = default;

		constexpr Override(Override&& other) noexcept {
			std::swap(ref, other.ref);
		}
		constexpr Override(const Override &other) : Override(*other.ref){}

		constexpr ~Override() noexcept{
			if(!ref) { return; }
			--ref->overrides;
		}

		constexpr Override& operator =(Override &&other) noexcept{
			std::swap(ref, other.ref);
			return *this;
		}

		constexpr Override& operator =(const Override &other) noexcept{
			if(this == &other) { return *this; }
			ref = other.ref;
			++ref->overrides;
			return *this;
		}

		constexpr operator bool() const {
			return ref;
		}
	};

	[[nodiscard]] Override getOverride(){
		return Override(*this);
	}
};

class window_handler {
	std::unique_ptr<MID3SMPS::main_window> mainWindow;
	ImGuiWrapConfig config_;
	FpsIdling idling_;

public:

	[[nodiscard]] const ImGuiWrapConfig& config() const noexcept;
	[[nodiscard]] FpsIdling& idling() noexcept;

	window_handler();
	static void MainLoopInit();
	ImGuiWrapperReturnType MainLoopStep();

	void IdleBySleeping();
};

namespace MID3SMPS {
	static window_handler handler;
}
#include "backend/window_handler.hpp"
#include "windows/main_window.hpp"

int main() {
	return imgui_main(
		MID3SMPS::handler.config(),
		[] {
			return MID3SMPS::handler.MainLoopStep();
		},
		[] {
			window_handler::MainLoopInit();
		}
	);
}

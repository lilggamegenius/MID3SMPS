#include "backend/window_handler.hpp"
#include "windows/main_window.hpp"

int main() {
	static window_handler handler;
	return imgui_main(
		handler.config(),
		[] {
			return handler.MainLoopStep();
		},
		[] {
			window_handler::MainLoopInit();
		}
	);
}

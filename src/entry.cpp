#include "backend/window_handler.hpp"

int main() {
	return imgui_main(
		MID3SMPS::handler.config(),
		[] {
			return MID3SMPS::handler.main_loop_step();
		},
		[] {
			MID3SMPS::handler.main_loop_init();
		}
	);
}

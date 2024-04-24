#include <imguiwrap.dear.h>
#include <imguiwrap.h>
#include <imgui_internal.h>
#include <GLFW/glfw3.h>

#include "gui/backend/window_handler.hpp"
#include "containers/program_persistence.hpp"
#include "gui/windows/main_window.hpp"

namespace MID3SMPS {
	bool show_demo_window = true;
	window_handler handler;
}

void window_handler::main_loop_init() {
	// Add .ini handle for UserData type
	ImGuiSettingsHandler ini_handler;
	ini_handler.TypeName   = MID3SMPS::program_persistence::TypeName;
	ini_handler.TypeHash   = ImHashStr(MID3SMPS::program_persistence::TypeName);
	ini_handler.ReadOpenFn = MID3SMPS::program_persistence::read_open;
	ini_handler.ReadLineFn = MID3SMPS::program_persistence::read_line;
	ini_handler.WriteAllFn = MID3SMPS::program_persistence::write_all;
	ImGui::GetCurrentContext()->SettingsHandlers.push_back(ini_handler);

	auto &style = ImGui::GetStyle();
	style.ItemInnerSpacing = dear::Zero;
	style.ItemSpacing = {0, 2};
	style.FramePadding = {4, 1};
	style.CellPadding = dear::Zero;

	reload_fonts();
}

[[nodiscard]] /*consteval*/ ImFontConfig window_handler::generate_font_config() {
	ImFontConfig cfg;
	cfg.OversampleH = cfg.OversampleV = 3;
	return cfg;
}

void window_handler::reload_fonts(const float pixel_size) {
	const auto fonts = ImGui::GetIO().Fonts;
	fonts->ClearFonts();
	static /*constexpr*/ auto cfg = generate_font_config();
	#define FOLDER_PATH "data/fonts/" // macro because there's no way to concat strings at compile-time
	main_font_ = fonts->AddFontFromFileTTF(FOLDER_PATH "SourceCodePro-Semibold.ttf", pixel_size, &cfg);
	main_font_bold_ = fonts->AddFontFromFileTTF(FOLDER_PATH "SourceCodePro-Black.ttf", pixel_size, &cfg);
	#undef FOLDER_PATH
}

ImGuiWrapperReturnType window_handler::main_loop_step(){
	idle_by_sleeping();
#ifdef DEBUG
	if(MID3SMPS::show_demo_window){
		ImGui::ShowDemoWindow(&MID3SMPS::show_demo_window);
	}
#endif
	mainWindow->render();
	mainWindow->render_children();
	if(mainWindow->keep()){
		return std::nullopt;
	}
	mainWindow->on_close();
	return 0;
}

// Init code
window_handler::window_handler(){
	config_.enableVsync_ = true;
	config_.windowTitle_ = "MID3SMPS";
	config_.enableDocking_ = true;
	config_.enableViewport_ = true;
	config_.enableViewportAutoMerge_ = false;
	config_.hideMainWindow_ = true;

	mainWindow = std::make_unique<MID3SMPS::main_window>();
}

void window_handler::idle_by_sleeping(){
	constexpr uint_fast8_t framesToWait = 2;
	static uint_fast8_t framesBeforeIdle = framesToWait;
	idling_.isIdling = false;
	if(idling_.is_idle_override()){
		framesBeforeIdle = framesToWait;
		return;
	}
	if(framesBeforeIdle > 0){
		framesBeforeIdle--;
	}
	if ((idling_.fpsIdle > 0) && idling_.enableIdling){
		using namespace std::chrono;
		const auto beforeWait = system_clock::now();
		//double waitTimeout = 1. / static_cast<double>(idling_.fpsIdle);
		const double waitTimeout = 1. / static_cast<double>(idling_.fpsIdle);

		// Backend specific call that will wait for an event for a maximum duration of waitTimeout
		// (for example glfwWaitEventsTimeout(timeout_seconds))
		glfwWaitEventsTimeout(waitTimeout);

		const auto afterWait = system_clock::now();
		const auto waitDuration = afterWait - beforeWait;
		const milliseconds waitIdleExpected(1000 / idling_.fpsIdle);
		idling_.isIdling = (waitDuration > waitIdleExpected * 0.9);
	}
}

const ImGuiWrapConfig &window_handler::config() const noexcept{
	return config_;
}

FpsIdling& window_handler::idling() noexcept{
	return idling_;
}


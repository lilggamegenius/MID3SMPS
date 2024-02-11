#include "backend/window_handler.hpp"
#include "windows/main_window.hpp"
#include "containers/program_persistence.hpp"

#include <GLFW/glfw3.h>
#include <imguiwrap.h>
#include <imgui_internal.h>

void window_handler::MainLoopInit() {
	// Add .ini handle for UserData type
	ImGuiSettingsHandler ini_handler;
	ini_handler.TypeName   = MID3SMPS::program_persistence::TypeName;
	ini_handler.TypeHash   = ImHashStr(MID3SMPS::program_persistence::TypeName);
	ini_handler.ReadOpenFn = MID3SMPS::program_persistence::read_open;
	ini_handler.ReadLineFn = MID3SMPS::program_persistence::read_line;
	ini_handler.WriteAllFn = MID3SMPS::program_persistence::write_all;
	ImGui::GetCurrentContext()->SettingsHandlers.push_back(ini_handler);
}

ImGuiWrapperReturnType window_handler::MainLoopStep(){
	IdleBySleeping();
#ifdef DEBUG
	static bool show_demo_window = true;

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if(show_demo_window){
		ImGui::ShowDemoWindow(&show_demo_window);
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

	mainWindow = std::make_unique<MID3SMPS::main_window>(*this);
}

void window_handler::IdleBySleeping(){
	constexpr uint_fast8_t framesToWait = 2;
	static uint_fast8_t framesBeforeIdle = framesToWait;
	idling_.isIdling = false;
	if(idling_.isIdleOverride()){
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

		const auto afterWait    = system_clock::now();
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


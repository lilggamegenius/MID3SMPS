#include "backend/WindowHandler.hpp"
#include "windows/MainWindow.hpp"
#include "GLFW/glfw3.h"
#include <imguiwrap.h>


int main(){
	auto handler = std::make_unique<WindowHandler>();
	return imgui_main(handler->config(), [&handler]{ return handler->MainLoopStep(); });
}

ImGuiWrapperReturnType WindowHandler::MainLoopStep(){
	IdleBySleeping();
#ifdef DEBUG
	static bool show_demo_window = true;

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if(show_demo_window){
		ImGui::ShowDemoWindow(&show_demo_window);
	}
#endif
	mainWindow->render();
	mainWindow->renderChildren();
	if(mainWindow->keep()){
		return std::nullopt;
	}
	mainWindow->onClose();
	return 0;
}

// Init code
WindowHandler::WindowHandler(){
	config_.enableVsync_ = true;
	config_.windowTitle_ = "Mid3SMPS";
	config_.enableDocking_ = true;
	config_.enableViewport_ = true;
	config_.enableViewportAutoMerge_ = false;
	config_.hideMainWindow_ = true;

	mainWindow = std::make_unique<MID3SMPS::MainWindow>(*this);
}

void WindowHandler::IdleBySleeping(){
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
		const auto waitDuration = (afterWait - beforeWait);
		const milliseconds waitIdleExpected(1000 / idling_.fpsIdle);
		idling_.isIdling = (waitDuration > waitIdleExpected * 0.9);
	}
}

const ImGuiWrapConfig &WindowHandler::config() const noexcept{
	return config_;
}

FpsIdling& WindowHandler::idling() noexcept{
	return idling_;
}


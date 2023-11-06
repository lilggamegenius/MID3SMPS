#include <fmt/core.h>

#include "backend/WindowHandler.hpp"
#include "windows/MainWindow.hpp"
#include "GLFW/glfw3.h"
#include <imguiwrap.h>
#include <imguiwrap.dear.h>

int main(){
	WindowHandler handler;
	return imgui_main(handler.config(), [&handler]{ return handler.MainLoopStep(); });
}

ImGuiWrapperReturnType WindowHandler::MainLoopStep(){
	IdleBySleeping();
	// Our state
#ifdef DEBUG
	// (we use static, which essentially makes the variable globals, as a convenience to keep the example code easy to follow)
	static bool show_demo_window = true;

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if(show_demo_window){
		ImGui::ShowDemoWindow(&show_demo_window);
	}
#endif
	mainWindow->render();

	for(auto iter = windowList.begin(); iter != windowList.end();){
		auto win = *iter;
		try{
			win->render();
			if(win->keep()){
				++iter;
				continue;
			}
		} catch(std::runtime_error &error){
			fmt::print(stderr, "std::runtime_error: {}", error.what());
		}catch(std::exception &error){
			fmt::print(stderr, "std::exception: {}", error.what());
		}
		win->onClose();
		iter = windowList.erase(iter); // reset iterator to a valid value post-erase
	}
	if(mainWindow->keep()){
		return std::nullopt;
	}
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

	mainWindow = std::make_shared<MID3SMPS::MainWindow>(*this);
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
		auto beforeWait = system_clock::now();
		//double waitTimeout = 1. / static_cast<double>(idling_.fpsIdle);
		double waitTimeout = 1. / static_cast<double>(idling_.fpsIdle);

		// Backend specific call that will wait for an event for a maximum duration of waitTimeout
		// (for example glfwWaitEventsTimeout(timeout_seconds))
		glfwWaitEventsTimeout(waitTimeout);

		auto afterWait = system_clock::now();
		auto waitDuration = (afterWait - beforeWait);
		milliseconds waitIdleExpected(1000 / idling_.fpsIdle);
		idling_.isIdling = (waitDuration > waitIdleExpected * 0.9);
	}
}

const ImGuiWrapConfig &WindowHandler::config() const noexcept{
	return config_;
}

FpsIdling& WindowHandler::idling() noexcept{
	return idling_;
}


#include <fmt/core.h>

#include "backend/WindowHandler.hpp"
#include "windows/MainWindow.hpp"
#include <imguiwrap.h>
#include <imguiwrap.dear.h>

int main(){
	WindowHandler handler;
	return imgui_main(handler.Config(), [&handler]{ return handler.MainLoopStep(); });
}

ImGuiWrapperReturnType WindowHandler::MainLoopStep(){
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
	config.enableVsync_ = true;
	config.windowTitle_ = "Mid3SMPS";
	config.enableDocking_ = true;
	config.enableViewport_ = true;
	config.enableViewportAutoMerge_ = false;
	config.hideMainWindow_ = true;

	mainWindow = std::make_shared<MID3SMPS::MainWindow>(*this);
}

const ImGuiWrapConfig &WindowHandler::Config() const{
	return config;
}


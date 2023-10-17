#pragma once

#include <list>
#include <memory>
#include "windows/Window.hpp"
#include <imguiwrap.h>

class WindowHandler {
	std::list<std::shared_ptr<Window>> windowList;
	std::shared_ptr<Window> mainWindow;
	ImGuiWrapConfig config;

public:
	template<typename Window>
	void addWindow(Window&& window) {
		windowList.push_back(std::make_shared<Window>(window));
	}

	template<typename Window>
	void removeWindow(const Window& window) {
		std::shared_ptr<::Window> winPtr;
		for (auto &win: windowList) {
			if(win.get() != std::addressof(window)) continue;
			winPtr = win;
			break;
		}

		if(winPtr){
			windowList.remove(winPtr);
		}
	}

	[[nodiscard]] const ImGuiWrapConfig& Config() const;

	WindowHandler();
	ImGuiWrapperReturnType MainLoopStep();
};
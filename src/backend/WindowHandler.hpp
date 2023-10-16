#pragma once

#include <list>
#include <memory>
#include "../Windows/window.hpp"

class WindowHandler {
	std::list<std::shared_ptr<Window>> windowList;
	std::weak_ptr<Window> mainWindow;

public:
	template<typename Window>
	void addWindow(Window&& window, bool isMainWindow = false) {
		windowList.push_back(std::make_shared<Window>(window));
		if(isMainWindow){
			mainWindow = windowList.back();
		}
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

	int startGUI();
	void MainLoopStep();
};
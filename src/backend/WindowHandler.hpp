#pragma once

#include <list>
#include <memory>
#include "../Windows/window.hpp"

class WindowHandler {
	std::list<std::unique_ptr<Window>> windowList;

public:
	template<typename Window>
	void addWindow(Window&& window) {
		windowList.push_back(std::make_unique<Window>(window));
	}

	template<typename Window>
	void removeWindow(const Window& window) {
		std::unique_ptr<::Window>* winPtr;
		for (auto &win: windowList) {
			if(win.get() != std::addressof(window)) continue;
			winPtr = &win;
			break;
		}

		if(winPtr){
			windowList.remove(*winPtr);
		}
	}

	int startGUI();
	void MainLoopStep();
};
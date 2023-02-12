#pragma once

class Window {

public:
	virtual ~Window() = default;
	// Returns true if we should keep the window
	virtual bool render() = 0;
	virtual void onClose() = 0;
};
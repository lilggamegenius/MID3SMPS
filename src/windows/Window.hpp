#pragma once

class Window {

public:
	virtual ~Window() = default;
	// Returns true if we should keep the window
	virtual void render() = 0;
	virtual void onClose() = 0;
	[[nodiscard]] virtual bool keep() const = 0;
};
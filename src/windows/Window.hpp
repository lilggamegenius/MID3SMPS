#pragma once

class Window {

public:
	virtual ~Window() = default;
	virtual void render() = 0;
	virtual void renderChildren(){}
	virtual void onClose() = 0;
	[[nodiscard]] virtual bool keep() const = 0;
};
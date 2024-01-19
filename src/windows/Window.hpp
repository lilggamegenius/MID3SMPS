#pragma once

template<typename Derived>
class Window {
	[[nodiscard]] constexpr Derived& derived() {
		return *static_cast<Derived*>(this);
	}

	[[nodiscard]] constexpr const Derived& derived() const {
		return *static_cast<const Derived*>(this);
	}

public:
	void render() {
		derived().render_impl();
	}

	void render_children() {
		derived().render_children_impl();
	}

	void on_close() {
		derived().on_close_impl();
	}

	[[nodiscard]] bool keep() const {
		return derived().keep_impl();
	}

protected:
	Window() = default;
	Window(const Window&) = default;
	Window(Window&&) = default;
};
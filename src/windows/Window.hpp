#pragma once

#include <unordered_map>

template<typename Derived>
class window {
	[[nodiscard]] constexpr Derived& derived() {
		return *static_cast<Derived*>(this);
	}

	[[nodiscard]] constexpr const Derived& derived() const {
		return *static_cast<const Derived*>(this);
	}

protected:
	using cached_key_t = const void*;
	std::unordered_map<cached_key_t, std::string> cached_strings{};

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

	[[nodiscard]] constexpr const char* window_title() const{
		return derived().window_title_impl();
	}

	std::string& cache_string(const cached_key_t ptr, const std::string &str) {
		auto [iter, _] = cached_strings.insert_or_assign(ptr, str);
		return iter->second;
	}

	std::string& cache_string(const cached_key_t ptr, std::string &&str) {
		auto [iter, _] = cached_strings.insert_or_assign(ptr, std::move(str));
		return iter->second;
	}

	std::string& cache_string(const cached_key_t ptr) {
		return cached_strings[ptr];
	}

protected:
	window() = default;
	window(const window&) = default;
	window(window&&) = default;
};
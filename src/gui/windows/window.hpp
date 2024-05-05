#pragma once

#include <string>
#include <unordered_map>

class window {
protected:
	using cached_key_t = const void*;
	std::unordered_map<cached_key_t, std::string> cached_strings{};

	bool stay_open_ = true;

public:
	virtual void render() = 0;

	virtual void render_children() {}

	virtual void on_close() = 0;

	[[nodiscard]] virtual bool keep() const {
		return stay_open_;
	}

	[[nodiscard]] constexpr virtual const char* window_title() const = 0;

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
	window(window&&) = default;
public:
	window(const window&) = delete;
	virtual ~window() = default;
};
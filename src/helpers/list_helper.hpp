#pragma once

#include <span>

namespace MID3SMPS {
	template<typename T>
	struct list_helper {
		using type = std::span<const T>;
		[[nodiscard, gnu::const]] static constexpr auto list() -> type = delete;
	};

	template<typename T>
	static constexpr auto list() {
		return list_helper<T>::list();
	}
}
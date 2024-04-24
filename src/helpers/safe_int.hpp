#pragma once

#include <concepts>
#include <cstdint>
#include <stdexcept>

namespace MID3SMPS {
	template<std::integral From, std::integral To>
	struct safe_promote {
		static constexpr bool same_sign          = std::is_unsigned_v<From> == std::is_unsigned_v<To>;
		static constexpr bool larger             = sizeof(To) > sizeof(From);
		static constexpr bool unsigned_to_signed = std::is_unsigned_v<From> && std::is_signed_v<To>;
		static constexpr bool value              = larger && (same_sign || unsigned_to_signed);
	};
	template<std::integral From, std::integral To>
	inline constexpr bool safe_promote_v = safe_promote<From, To>::value;

	static_assert(safe_promote_v<std::uint8_t, std::uint16_t>);
	static_assert(safe_promote_v<std::uint8_t, std::int16_t>);
	static_assert(!safe_promote_v<std::uint8_t, std::int8_t>);
	static_assert(!safe_promote_v<std::uint16_t, std::uint8_t>);

	// ReSharper disable CppNonExplicitConversionOperator
	// ReSharper disable CppNonExplicitConvertingConstructor
	template<std::integral Int>
	struct safe_int {
		using value_type = std::remove_cvref_t<Int>;

		value_type value{};

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int(T underlying_) noexcept : value(static_cast<value_type>(underlying_)) {}

		template<std::integral T>
		constexpr explicit(!safe_promote_v<T, value_type>) safe_int(const safe_int<T> &other) noexcept {
			value = static_cast<value_type>(other.value); // Promote safely OR explicitly
		}

		constexpr safe_int(value_type underlying_) noexcept : value(underlying_) {}
		constexpr safe_int() = default;

		// Allow using type as array index
		constexpr operator std::size_t() const noexcept(safe_promote_v<value_type, std::size_t>) {
			if constexpr(!safe_promote_v<value_type, std::size_t>) {
				if(value < 0) {
					throw std::logic_error("Attempted to convert negative value to array index");
				}
			}
			return static_cast<std::size_t>(value);
		}

		constexpr explicit operator value_type() const noexcept {
			return value;
		}

		constexpr explicit operator bool() const noexcept {
			return value;
		}

		constexpr safe_int operator+(const safe_int &rhs) const {
			return static_cast<value_type>(value + rhs.value);
		}

		constexpr safe_int operator-(const safe_int &rhs) const {
			return static_cast<value_type>(value - rhs.value);
		}

		constexpr safe_int operator*(const safe_int &rhs) const {
			return static_cast<value_type>(value * rhs.value);
		}

		constexpr safe_int operator/(const safe_int &rhs) const {
			return static_cast<value_type>(value / rhs.value);
		}

		constexpr safe_int operator%(const safe_int &rhs) const {
			return static_cast<value_type>(value % rhs.value);
		}

		constexpr safe_int operator&(const safe_int &rhs) const {
			return static_cast<value_type>(value & rhs.value);
		}

		constexpr safe_int operator^(const safe_int &rhs) const {
			return static_cast<value_type>(value ^ rhs.value);
		}

		constexpr safe_int operator|(const safe_int &rhs) const {
			return static_cast<value_type>(value | rhs.value);
		}

		constexpr safe_int operator<<(const safe_int &rhs) const {
			return static_cast<value_type>(value << rhs.value);
		}

		constexpr safe_int operator>>(const safe_int &rhs) const {
			return static_cast<value_type>(value >> rhs.value);
		}

		constexpr safe_int operator~() const {
			return static_cast<value_type>(~value);
		}

		constexpr safe_int &operator++() {
			++value;
			return *this;
		}

		constexpr safe_int &operator--() {
			--value;
			return *this;
		}

		constexpr safe_int operator++(int) {
			const auto copy = *this;
			++value;
			return copy;
		}

		constexpr safe_int operator--(int) {
			const auto copy = *this;
			--value;
			return copy;
		}

		constexpr safe_int &operator +=(const safe_int &rhs) {
			*this = this->operator+(rhs);
			return *this;
		}

		constexpr safe_int &operator -=(const safe_int &rhs) {
			*this = this->operator-(rhs);
			return *this;
		}

		constexpr safe_int &operator *=(const safe_int &rhs) {
			*this = this->operator*(rhs);
			return *this;
		}

		constexpr safe_int &operator /=(const safe_int &rhs) {
			*this = this->operator/(rhs);
			return *this;
		}

		constexpr safe_int &operator %=(const safe_int &rhs) {
			*this = this->operator%(rhs);
			return *this;
		}

		constexpr safe_int &operator &=(const safe_int &rhs) {
			*this = this->operator&(rhs);
			return *this;
		}

		constexpr safe_int &operator ^=(const safe_int &rhs) {
			*this = this->operator^(rhs);
			return *this;
		}

		constexpr safe_int &operator |=(const safe_int &rhs) {
			*this = this->operator|(rhs);
			return *this;
		}

		constexpr safe_int &operator <<=(const safe_int &rhs) {
			*this = this->operator<<(rhs);
			return *this;
		}

		constexpr safe_int &operator >>=(const safe_int &rhs) {
			*this = this->operator>>(rhs);
			return *this;
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator+(const T &rhs) const {
			return static_cast<value_type>(value + static_cast<value_type>(rhs));
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator-(const T &rhs) const {
			return static_cast<value_type>(value - static_cast<value_type>(rhs));
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator*(const T &rhs) const {
			return static_cast<value_type>(value * static_cast<value_type>(rhs));
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator/(const T &rhs) const {
			return static_cast<value_type>(value / static_cast<value_type>(rhs));
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator%(const T &rhs) const {
			return static_cast<value_type>(value % static_cast<value_type>(rhs));
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator&(const T &rhs) const {
			return static_cast<value_type>(value & static_cast<value_type>(rhs));
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator^(const T &rhs) const {
			return static_cast<value_type>(value ^ static_cast<value_type>(rhs));
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator|(const T &rhs) const {
			return static_cast<value_type>(value | static_cast<value_type>(rhs));
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator<<(const T &rhs) const {
			return static_cast<value_type>(value << static_cast<value_type>(rhs));
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator>>(const T &rhs) const {
			return static_cast<value_type>(value >> static_cast<value_type>(rhs));
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator <<=(const T &rhs) {
			value = static_cast<value_type>(value << static_cast<value_type>(rhs));
			return value;
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr safe_int operator >>=(const T &rhs) {
			value = static_cast<value_type>(value >> static_cast<value_type>(rhs));
			return value;
		}

		// Boilerplate
		constexpr safe_int(const safe_int &other) : value(other.value) {}
		constexpr safe_int(safe_int &&other) noexcept : value(std::move(other.value)) {}

		constexpr safe_int &operator=(const safe_int &other) {
			if(this == &other)
				return *this;
			value = other.value;
			return *this;
		}

		constexpr safe_int &operator=(safe_int &&other) noexcept {
			if(this == &other)
				return *this;
			value = std::move(other.value);
			return *this;
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr auto operator<=>(const T &rhs) const {
			return value <=> static_cast<value_type>(rhs);
		}

		constexpr auto operator<=>(const safe_int &rhs) const {
			return value <=> rhs.value;
		}

		constexpr friend bool operator==(const safe_int &lhs, const safe_int &rhs) {
			return (lhs <=> rhs) == std::strong_ordering::equal;
		}

		constexpr friend bool operator!=(const safe_int &lhs, const safe_int &rhs) {
			return !(lhs == rhs);
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr friend bool operator==(const safe_int &lhs, const T &rhs) {
			return (lhs <=> rhs) == std::strong_ordering::equal;
		}

		template<std::integral T> requires std::is_convertible_v<T, value_type>
		constexpr friend bool operator!=(const safe_int &lhs, const T &rhs) {
			return !(lhs == rhs);
		}
	};
	// ReSharper restore CppNonExplicitConversionOperator
	// ReSharper restore CppNonExplicitConvertingConstructor
}


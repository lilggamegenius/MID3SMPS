#pragma once

#include <cstdint>
#include <span>
#include <spanstream>

#include "containers/fm_instrument.hpp"

namespace MID3SMPS::M2S {
	enum class version {
		v1,
		v2,
		v3
	};

	struct options {
		bool chord_notes: 1;
	};

	struct chords {
		std::uint8_t note_count;
		std::span<std::int8_t> relative_notes;
	};

	namespace fm {
		struct patch : fm_instrument{
			M2S::options options{};
			chords chord_notes{};
			union {
				std::int8_t instrument_transposition;
				std::uint8_t default_drum_note{};
			};

			constexpr patch()									= default;
			constexpr patch(const patch &other)					= default;
			constexpr patch(patch &&other) noexcept				= default;
			constexpr patch& operator=(const patch &other)		= default;
			constexpr patch& operator=(patch &&other) noexcept	= default;

		private:
			void load_v1(std::span<const std::uint8_t> data) { (void)data; throw std::runtime_error("Version 1 FM data not yet supported"); }
			void load_v2(std::span<const std::uint8_t> data) { (void)data; throw std::runtime_error("Version 2 FM data not yet supported"); }
			void load_v3(std::span<const std::uint8_t> data);

		public:
			constexpr patch(version version, std::span<const std::uint8_t> data) {
				switch(version) {
					case version::v1:
						load_v1(data);
						break;
					case version::v2:
						load_v2(data);
						break;
					case version::v3:
						load_v3(data);
						break;
					default:
						throw std::logic_error("Invalid instrument");
				}
			}

			constexpr ~patch() override = default;
		};

		static patch empty_patch{};
	}

	template<typename T>
	constexpr T span_convert(std::span<const std::uint8_t> data) {
		static_assert(std::is_trivially_copyable_v<T>);
		static constexpr auto size = sizeof(T);
		if(data.size() < size) {
			throw std::runtime_error(fmt::format("Invalid data size: Expected {}, got {}", size, data.size()));
		}
		T ret;
		static constexpr auto native = std::endian::native;
		if constexpr (native == std::endian::little) {
			std::memcpy(&ret, data.data(), size);
		} else if constexpr (native == std::endian::big) {
			static_assert(false, "Big endian not supported yet."); // Todo
		} else {
			static_assert(false, "Mixed endian not supported");
		}
		return ret;
	}

	template<typename T>
	constexpr T stream_convert(std::basic_ispanstream<std::uint8_t> &stream) {
		static_assert(std::is_trivially_copyable_v<T>);
		constexpr auto size = sizeof(T);
		std::array<std::uint8_t, size> data{};

		static constexpr auto native = std::endian::native;
		if constexpr (native == std::endian::little) {
			stream.read(data.data(), size);
		} else if constexpr (native == std::endian::big) {
			static_assert(false, "Big endian not supported yet."); // Todo: Impliment this when it actually becomes an issue
		} else {
			static_assert(false, "Mixed endian not supported");
		}
		return std::bit_cast<T>(data);
	}
}

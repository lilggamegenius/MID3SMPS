#include "patch.hpp"

#include <spanstream>

namespace MID3SMPS::fm {
	constexpr void patch::load_v3(std::span<const std::uint8_t> data) {
		std::basic_ispanstream stream(std::bit_cast<std::span<std::uint8_t>>(data), std::ios::binary);
		const auto total_size = stream_convert<std::uint16_t>(stream);
		if(total_size != data.size()) {
			throw std::runtime_error(fmt::format("Instrument data size does not match included size: Length:{:#04X} != TotalSize:{:#04X}", data.size(), total_size));
		}
		stream.read(operators.registers.data(), static_cast<std::streamsize>(operators.registers.size()));
		instrument_transposition = stream_convert<std::int8_t>(stream);
		options = stream_convert<struct options>(stream);
		if(options.chord_notes) {
			throw std::logic_error("chord notes not implimented yet"); // todo
		}

		const auto name_length = stream_convert<std::uint8_t>(stream);
		name.resize(name_length);
		stream.read(reinterpret_cast<std::uint8_t*>(name.data()), name_length);
	}
} // MID3SMPS
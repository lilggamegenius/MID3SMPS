#include "gyb.hpp"

#include <fstream>
#include <spanstream>

namespace MID3SMPS {
	namespace errors {
		static constexpr auto invalid = "Not a valid GYB formatted file";
		static constexpr auto missing = "GYB file does not exist";
	}

	gyb::gyb(const fs::path &path) {
		using byte_t = std::uint8_t;
		std::vector<byte_t> data;
		if(exists(path)) {
			std::ifstream file(path, std::ios::binary);
			file.unsetf(std::ios::skipws);

			file.seekg(0, std::ios::end);
			const auto fileSize = file.tellg();
			file.seekg(0, std::ios::beg);

			data.resize(static_cast<std::size_t>(fileSize));

			file.read(reinterpret_cast<std::ifstream::char_type*>(&data.front()), fileSize);
		} else {
			throw std::runtime_error(errors::missing);
		}

		if(data[0] != 26 || data[1] != 12) {
			throw std::runtime_error(errors::invalid);
		}

		switch(data[2]) {
			case 1:
				load_v1(data);
				break;
			case 2:
				load_v2(data);
				break;
			case 3:
				load_v3(data);
				break;
			default:
				throw std::runtime_error(errors::invalid);
		}
	}

	void gyb::load_v1(std::span<const std::uint8_t> data) {
		(void)data;
		default_LFO_speed = lfo::off;
		throw std::logic_error("Not inplimented yet"); // Todo
	}
	void gyb::load_v2(std::span<const std::uint8_t> data) {
		default_LFO_speed = static_cast<lfo>(data[105]);
		throw std::logic_error("Not inplimented yet"); // Todo
	}
	void gyb::load_v3(std::span<const std::uint8_t> data) {
		static constexpr auto version = version::v3;
		std::basic_ispanstream stream(std::bit_cast<std::span<std::uint8_t>>(data), std::ios::binary);
		stream.seekg(3);
		default_LFO_speed = stream_convert<lfo>(stream);
		if(const auto filesize = stream_convert<std::uint32_t>(stream); filesize != data.size()) {
			throw std::runtime_error(errors::invalid);
		}
		const auto bank_offset = stream_convert<std::uint32_t>(stream);
		const auto maps_offset = stream_convert<std::uint32_t>(stream); (void)maps_offset; // Todo impliment reading mappings
		stream.seekg(bank_offset);
		const auto instrument_count = stream_convert<std::uint16_t>(stream);
		patches.reserve(instrument_count);
		auto &[melodic_name, melodic_order] = patches_order[bank::melodic];
		melodic_name = "Melodic bank";
		melodic_order.reserve(instrument_count);
		for(ins_key_t current_instrument = 0; current_instrument < instrument_count; current_instrument++) {
			const auto start_of_instrument = stream.tellg();
			const auto instrument_size = stream_convert<std::uint16_t>(stream);
			add_patch(bank::melodic, version, data.subspan(static_cast<std::size_t>(start_of_instrument), instrument_size));
			stream.seekg(start_of_instrument + static_cast<std::streamoff>(instrument_size));
		}

		const auto drum_count = stream_convert<std::uint16_t>(stream);
		patches.reserve(drum_count);
		auto &[drum_name, drum_order] = patches_order[bank::drum];
		drum_name = "drum bank";
		drum_order.reserve(instrument_count + drum_count);
		for(ins_key_t current_instrument = 0; current_instrument < drum_count; current_instrument++) {
			const auto start_of_instrument = stream.tellg();
			const auto instrument_size = stream_convert<std::uint16_t>(stream);
			add_patch(bank::drum, version, data.subspan(static_cast<std::size_t>(start_of_instrument), instrument_size));
			stream.seekg(start_of_instrument + static_cast<std::streamoff>(instrument_size));
		}
	}
}

#include <fstream>

#include "mapping.hpp"
#include "exceptions/formatException.hpp"

namespace MID3SMPS::M2S {
	mapping::mapping(const fs::path &path) {
		std::ifstream input(path);
		input.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
		std::string line;
		std::getline(input, line);
		if(static constexpr std::string_view header = "-- mid2smps Configuration --"; header != line) {
			throw format_exception("File is not a mappings file");
		}

		const auto parent = path.parent_path();

		std::getline(input, line); gyb_		= parent / line;
		std::getline(input, line); dac_map_	= parent / line;
		std::getline(input, line); dac_list_	= parent / line;
		std::getline(input, line); psg_list_	= parent / line;
	}
}

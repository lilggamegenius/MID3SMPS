#pragma once

#include <filesystem>

namespace fs = std::filesystem;

namespace MID3SMPS {
	class mapping {
		fs::path gyb_;
		fs::path dac_map_;
		fs::path dac_list_;
		fs::path psg_list_;

	public:
		explicit mapping(const fs::path &path);

		[[nodiscard]] const fs::path &gyb() const {
			return gyb_;
		}

		void gyb(const fs::path &gyb) {
			gyb_ = gyb;
		}

		[[nodiscard]] const fs::path &dac_map() const {
			return dac_map_;
		}

		void dac_map(const fs::path &dac_map) {
			dac_map_ = dac_map;
		}

		[[nodiscard]] const fs::path &dac_list() const {
			return dac_list_;
		}

		void dac_list(const fs::path &dac_list) {
			dac_list_ = dac_list;
		}

		[[nodiscard]] const fs::path &psg_list() const {
			return psg_list_;
		}

		void psg_list(const fs::path &psg_list) {
			psg_list_ = psg_list;
		}

		// boilerplate
		mapping()                           = default;
		mapping(const mapping &)            = default;
		mapping(mapping &&)                 = default;
		mapping &operator=(const mapping &) = default;
		mapping &operator=(mapping &&)      = default;
		~mapping()                          = default;
	};
}

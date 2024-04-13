#pragma once

#include <filesystem>

namespace fs = std::filesystem;

namespace MID3SMPS {
	class mapping {
		fs::path gyb_{};
		fs::path dac_map_{};
		fs::path dac_list_{};
		fs::path psg_list_{};

	public:
		explicit mapping(const fs::path &path);

		[[nodiscard]] constexpr const fs::path &gyb() const {
			return gyb_;
		}

		void gyb(const fs::path &gyb) {
			gyb_ = gyb;
		}

		[[nodiscard]] constexpr const fs::path &dac_map() const {
			return dac_map_;
		}

		void dac_map(const fs::path &dac_map) {
			dac_map_ = dac_map;
		}

		[[nodiscard]] constexpr const fs::path &dac_list() const {
			return dac_list_;
		}

		void dac_list(const fs::path &dac_list) {
			dac_list_ = dac_list;
		}

		[[nodiscard]] constexpr const fs::path &psg_list() const {
			return psg_list_;
		}

		void psg_list(const fs::path &psg_list) {
			psg_list_ = psg_list;
		}

		// boilerplate
		mapping() = default;
		mapping(const mapping &map) {
			gyb_		= map.gyb_;
			dac_map_	= map.dac_map_;
			dac_list_	= map.dac_list_;
			psg_list_	= map.psg_list_;
		}
		mapping(mapping &&map) noexcept {
			gyb_		= std::move(map.gyb_);
			dac_map_	= std::move(map.dac_map_);
			dac_list_	= std::move(map.dac_list_);
			psg_list_	= std::move(map.psg_list_);
		}
		mapping &operator=(const mapping &map) { // NOLINT(*-use-equals-default)
			gyb_		= map.gyb_;
			dac_map_	= map.dac_map_;
			dac_list_	= map.dac_list_;
			psg_list_	= map.psg_list_;
			return *this;
		}
		mapping &operator=(mapping &&map) noexcept { // NOLINT(*-use-equals-default)
			gyb_		= std::move(map.gyb_);
			dac_map_	= std::move(map.dac_map_);
			dac_list_	= std::move(map.dac_list_);
			psg_list_	= std::move(map.psg_list_);
			return *this;
		}
		~mapping() = default;
	};
}

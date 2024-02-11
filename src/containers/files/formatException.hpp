#pragma once

#include <stdexcept>

namespace MID3SMPS {

class format_exception final : public std::runtime_error{
public:
	explicit format_exception(const std::string &arg) : runtime_error{arg} {}

	explicit format_exception(const char *string) : runtime_error{string} {}

	format_exception(format_exception &&error) noexcept : runtime_error{std::move(error)} {}

	format_exception(const format_exception &error) : runtime_error{error} {}
};

} // MID3SMPS
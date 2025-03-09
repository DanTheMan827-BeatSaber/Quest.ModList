#pragma once

#include "paper/shared/logger.hpp"

/// @brief A logger, useful for printing debug messages
/// @return
inline constexpr auto Logger = Paper::ConstLoggerContext(MOD_ID "_" VERSION);

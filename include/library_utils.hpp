#pragma once

#include <optional>
#include <string>
#include <unordered_map>

using LibraryLoadInfo = std::unordered_map<std::string, std::optional<std::string>>;

/**
 * @brief Loads shared library files (.so) from the specified directory.
 *
 * Iterates over the folder and attempts to load each library.
 *
 * @param path The directory path containing the libraries.
 * @return LibraryLoadInfo Mapping of library filenames to optional failure reasons (nullopt on success).
 */
LibraryLoadInfo GetLoadedLibraries(std::string const& path);

/**
 * @brief Gets the load information for modloader libraries.
 *
 * Provides access to the load statuses of modloader libraries.
 *
 * @return LibraryLoadInfo& Mapping of library filenames to optional failure reasons.
 */
LibraryLoadInfo& GetModloaderLibsLoadInfo();

/**
 * @brief Gets the load information for mods.
 *
 * Provides access to the load statuses of mods.
 *
 * @return LibraryLoadInfo& Mapping of mod filenames to optional failure reasons.
 */
LibraryLoadInfo& GetModsLoadInfo();

/**
 * @brief Gets the load information for early mods.
 *
 * Provides access to the load statuses of early mods.
 *
 * @return LibraryLoadInfo& Mapping of early mod filenames to optional failure reasons.
 */
LibraryLoadInfo& GetEarlyModsLoadInfo();

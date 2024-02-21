#pragma once

#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include "scotland2/shared/modloader.h"

using LibraryLoadInfo = std::unordered_map<std::string, std::optional<std::string>>;

// Loops over all the SO files in the folder path, and attempts to load them
// Keys are library file names (with .so included), values are fail reasons, or nullopt if loading succeeded
LibraryLoadInfo GetLoadedLibraries(const std::string& path);


// Gets (or finds) the libraries that failed to load in this instance of the game running
// Keys are library file names (with .so included), values are fail reasons (or nullopt if loading was successful)
LibraryLoadInfo& GetModloaderLibsLoadInfo();


// Gets the mods that failed to load in this instance of the game running
// Keys are mod SO file names (with .so included), values are fail reasons (or nullopt if loading was successful)
LibraryLoadInfo& GetModsLoadInfo();

// Gets the early mods that failed to load in this instance of the game running
// Keys are early mod SO file names (with .so included), values are fail reasons (or nullopt if loading was successful)
LibraryLoadInfo& GetEarlyModsLoadInfo();
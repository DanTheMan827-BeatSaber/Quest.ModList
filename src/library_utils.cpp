#include "library_utils.hpp"

#include <dirent.h>
#include <dlfcn.h>
#include <sys/types.h>

#include "fmt/format.h"
#include "logger.hpp"
#include "scotland2/shared/modloader.h"

std::unordered_map<std::string, std::optional<std::string>> GetLoadedLibraries(std::string const& path) {
    Logger.info("Checking for libraries in path: %s", path.c_str());
    std::unordered_map<std::string, std::optional<std::string>> result;

    CLoadResults modsLoad = modloader_get_all();

    for (int i = 0; i < modsLoad.size; i++) {
        CLoadResult& loadResult = modsLoad.array[i];

        switch (loadResult.result) {
            case CLoadResultEnum::LoadResult_Failed: {
                std::filesystem::path libraryFile(loadResult.failed.path);
                if (libraryFile.string().starts_with(path)) {
                    result[libraryFile.filename().string()] = loadResult.failed.failure;
                }
                break;
            }
            case CLoadResultEnum::MatchType_Loaded: {
                std::filesystem::path libraryFile(loadResult.loaded.path);
                if (libraryFile.string().starts_with(path)) {
                    result[libraryFile.filename().string()] = std::nullopt;
                }
                break;
            }
            default: {
            }
        }
    }

    return result;
}

static std::optional<LibraryLoadInfo> failedLibraries;
static std::optional<LibraryLoadInfo> failedMods;
static std::optional<LibraryLoadInfo> failedEarlyMods;

LibraryLoadInfo& GetModloaderLibsLoadInfo() {
    static std::string libsPath = fmt::format("{}/libs", modloader_get_files_dir());

    if (!failedLibraries.has_value()) {
        failedLibraries = GetLoadedLibraries(libsPath);
    }

    return *failedLibraries;
}

LibraryLoadInfo& GetModsLoadInfo() {
    static std::string modsPath = fmt::format("{}/mods", modloader_get_files_dir());

    if (!failedMods.has_value()) {
        failedMods.emplace(GetLoadedLibraries(modsPath));
    }

    return *failedMods;
}

LibraryLoadInfo& GetEarlyModsLoadInfo() {
    static std::string earlyModsPath = fmt::format("{}/early_mods", modloader_get_files_dir());

    if (!failedEarlyMods.has_value()) {
        failedEarlyMods.emplace(GetLoadedLibraries(earlyModsPath));
    }

    return *failedEarlyMods;
}

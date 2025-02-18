#include "main.hpp"

#include "autohooks/shared/hooks.hpp"
#include "bsml/shared/BSML.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "modInfo.hpp"
#include "ModListViewController.hpp"
using namespace ModList;

/// @brief Called at the early stages of game loading
/// @param info The mod info.  Update this with your mod's info.
/// @return
MOD_EXPORT_FUNC void setup(CModInfo& info) {
    // Convert the mod info to a C struct and set that as the modloader info.
    info = modInfo.to_c();

    // Initialize the config
    getConfig().Init(modInfo);

    Logger.info("Completed setup!");
}

/// @brief Called early on in the game loading
/// @return
MOD_EXPORT_FUNC void load() {
    // Initialize il2cpp functions
    il2cpp_functions::Init();

    // Get the number of early hooks that will be installed.
    auto earlyHookCount = EARLY_HOOK_COUNT;

    // install early hooks
    if (earlyHookCount > 0) {
        Logger.info("Installing {} early hook{}", earlyHookCount, earlyHookCount == 0 || earlyHookCount > 1 ? "s" : "");
        INSTALL_EARLY_HOOKS();
        Logger.info("Finished installing early hook{}", earlyHookCount == 0 || earlyHookCount > 1 ? "s" : "");
    }
}

/// @brief Called later on in the game loading - a good time to install function hooks
/// @return
MOD_EXPORT_FUNC void late_load() {
    // Register our mod settings menu
    BSML::Init();
    custom_types::Register::AutoRegister();
    BSML::Register::RegisterMainMenu<ModListViewController*>("Loaded Mods", "Loaded Mods", "View Loaded Mods");
    BSML::Register::RegisterSettingsMenu("Mod List", ConfigViewDidActivate, true);

    // Get the number of late hooks that will be installed.
    auto lateHookCount = LATE_HOOK_COUNT;

    // Install late hooks
    if (lateHookCount > 0) {
        Logger.info("Installing {} late hook{}", lateHookCount, lateHookCount > 1 ? "s" : "");
        INSTALL_LATE_HOOKS();
        Logger.info("Finished installing late hook{}", lateHookCount == 0 || lateHookCount > 1 ? "s" : "");
    }
}

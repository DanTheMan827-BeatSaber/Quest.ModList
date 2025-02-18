#include "config-utils/shared/config-utils.hpp"
#include "HMUI/ViewController.hpp"

DECLARE_CONFIG(Config) {
    CONFIG_VALUE(showFailedOnStart, bool, "showFailedModsOnGameStart", true, "Show failed mods pop-up in main menu");
};

/**
 * @brief Activates the config view.
 *
 * @param self The view controller.
 * @param firstActivation Whether this is the first activation.
 * @param addedToHierarchy Whether the view was added to the hierarchy.
 * @param screenSystemEnabling Whether the screen system is enabling.
 */
void ConfigViewDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);

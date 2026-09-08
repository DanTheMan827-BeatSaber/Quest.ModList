#pragma once

#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"

/// @brief Declare a ViewController to let us create UI in the mods menu
DECLARE_CLASS_CODEGEN(ModList, ModListViewController, HMUI::ViewController) {
    /// @brief Override DidActivate, which is called whenever you enter the menu
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
};

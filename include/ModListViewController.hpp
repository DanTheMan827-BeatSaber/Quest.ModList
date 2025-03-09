#pragma once

#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"

/// @brief Declare a ViewController to let us create UI in the mods menu
DECLARE_CLASS_CODEGEN(ModList, ModListViewController, HMUI::ViewController,
    /// @brief Override DidActivate, which is called whenever you enter the menu
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
)

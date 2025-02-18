#include "autohooks/shared/hooks.hpp"
#include "config.hpp"
#include "library_utils.hpp"
#include "logger.hpp"

// BSML
#include "bsml/shared/BSML-Lite.hpp"
using namespace BSML;

// GlobalNamespace
#include "GlobalNamespace/MainMenuViewController.hpp"
using namespace GlobalNamespace;

// UnityEngine
#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/TextAnchor.hpp"
#include "UnityEngine/Transform.hpp"
using namespace UnityEngine;

// UnityEngine::UI
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
using namespace UnityEngine::UI;

// TMPro
#include "TMPro/TextAlignmentOptions.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
using namespace TMPro;

/**
 * @brief Draws a list of failed mods in the GUI.
 *
 * @param layout The layout to draw the list in.
 * @param failedMods The list of failed mods.
 * @param title The title of the list.
 */
void drawFailedList(VerticalLayoutGroup* layout, std::unordered_map<std::string, std::string> const& failedMods, std::string const& title) {
    if (failedMods.size() > 0) {
        // Create the title text for the failed mods
        TextMeshProUGUI* modsTitleText = Lite::CreateText(layout, title);
        modsTitleText->set_fontSize(5.0f);
        modsTitleText->set_alignment(TextAlignmentOptions::Top);
        modsTitleText->get_transform().cast<RectTransform>()->set_sizeDelta({70, 4});

        auto separator = Lite::CreateText(layout, "_____________________________________________________________________________________________");
        separator->set_alignment(TextAlignmentOptions::Bottom);
        separator->get_transform().cast<RectTransform>()->set_sizeDelta({70, 4});
        separator->set_overflowMode(TextOverflowModes::Overflow);
        ;

        // Add the failed mods to the GUI
        for (std::pair<std::string, std::string> failedMod : failedMods) {
            TextMeshProUGUI* modText = Lite::CreateText(layout, fmt::format("<color=red>{}</color>", failedMod.first.c_str()));
            modText->set_overflowMode(TextOverflowModes::Overflow);
            modText->set_fontSize(3.5f);
            modText->set_alignment(TextAlignmentOptions::Top);
            modText->get_transform().cast<RectTransform>()->get_transform().cast<RectTransform>()->set_sizeDelta({70, 3.5});

            Lite::AddHoverHint(
                modText, failedMod.second
            );  // Show the full fail reason in a hover hint, since there most likely won't be enough space in the modal view
        }

        Lite::CreateText(layout, " ")->get_transform().cast<RectTransform>()->set_sizeDelta({70, 1});
    }
}

// Displays a modal view if mods fail to load showing why
MAKE_LATE_HOOK_MATCH(
    MainMenuViewController_DidActivate,
    &MainMenuViewController::DidActivate,
    void,
    MainMenuViewController* self,
    bool firstActivation,
    bool addedToHierarchy,
    bool screenSystemEnabling
) {
    MainMenuViewController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    Logger.info("MainMenuViewController_DidActivate");
    if (!firstActivation) {
        Logger.info("Not first activation, not displaying modal");
        return;
    }

    // Check if we should show the failed mods on game start
    if (getConfig().showFailedOnStart.GetValue() == false) {
        Logger.info("Showing failed mods on game start is disabled! Returning");
        return;
    }

    // Check for failed mods
    Logger.info("Checking for failed mods . . .");
    auto modsLoadInfo = GetModsLoadInfo();
    auto earlyModsLoadInfo = GetEarlyModsLoadInfo();

    // Check if there are any failed mods
    auto failedMods = std::unordered_map<std::string, std::string>();
    for (auto modPair : modsLoadInfo) {
        if (modPair.second.has_value()) {
            failedMods[modPair.first] = *modPair.second;
        }
    }

    // Check if there are any failed early mods
    auto failedEarlyMods = std::unordered_map<std::string, std::string>();
    for (auto modPair : earlyModsLoadInfo) {
        if (modPair.second.has_value()) {
            failedEarlyMods[modPair.first] = *modPair.second;
        }
    }

    // Log the failed mods
    Logger.info("%lu mods failed to load", failedMods.size());
    Logger.info("%lu early mods failed to load", failedEarlyMods.size());

    // If there are no failed mods, don't show the modal
    if (failedMods.empty() && failedEarlyMods.empty()) {
        Logger.info("All mods loaded successfully, not showing fail dialog");

        return;
    }

    // Start constructing the fail dialog
    Logger.info("Constructing fail dialog . . .");

    // Create the modal view
    auto modalView = Lite::CreateModal(
        self->get_transform(),
        UnityEngine::Vector2(70.0f, 70.0f),
        [] {
        },
        true
    );

    // Destroy the modal view when it's hidden
    modalView->onHide = [modalView]() {
        Logger.info("Fail dialog closed, destroying modal view!");
        UnityEngine::GameObject::Destroy(modalView->get_gameObject());
    };

    // Create a scrollable container for the list
    UnityEngine::GameObject* scrollView = Lite::CreateScrollableSettingsContainer(modalView);
    auto scrollTransform = scrollView->get_transform().cast<RectTransform>();
    scrollTransform->set_sizeDelta({80, 70});

    // Create the vertical layout group
    auto layout = Lite::CreateVerticalLayoutGroup(scrollView);

    // Set the properties of the vertical layout group
    layout->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 0, 0));
    layout->set_spacing(0.5);
    layout->set_childAlignment(UnityEngine::TextAnchor::UpperCenter);
    layout->set_childControlHeight(false);
    layout->set_childControlWidth(false);

    // Create format the text for the failed mods
    std::string failedModsText;
    if (failedMods.size() > 1 || failedMods.size() == 0) {
        failedModsText = fmt::format("{} mods failed to load!", failedMods.size());
    } else {
        failedModsText = fmt::format("{} mod failed to load!", failedMods.size());
    }

    // Create the format the text for the failed early mods
    std::string failedEarlyModsText;
    if (failedEarlyMods.size() > 1 || failedEarlyMods.size() == 0) {
        failedEarlyModsText = fmt::format("{} early mods failed to load!", failedEarlyMods.size());
    } else {
        failedEarlyModsText = fmt::format("{} early mod failed to load!", failedEarlyMods.size());
    }

    // Add the failed mods to the GUI
    drawFailedList(layout, failedMods, failedModsText);

    // Add the failed early mods to the GUI
    drawFailedList(layout, failedEarlyMods, failedEarlyModsText);

    Lite::CreateUIButton(layout, "Close", [modalView]() {
        modalView->Hide();
    });

    // Show the modal view
    Logger.info("Showing fail dialog . . .");
    modalView->Show();
}

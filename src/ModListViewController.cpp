#include "ModListViewController.hpp"

#include "library_utils.hpp"
#include "logger.hpp"
using namespace ModList;

// UnityEngine
#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/TextAnchor.hpp"
using namespace UnityEngine;

// UnityEngine::UI
#include "UnityEngine/UI/HorizontalLayoutGroup.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
using namespace UnityEngine::UI;

// BSML
#include "bsml/shared/BSML-Lite.hpp"
using namespace BSML::Lite;

// TMPro
#include "TMPro/TextAlignmentOptions.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
using namespace TMPro;

// Scottland2
#include "scotland2/shared/modloader.h"

struct ListItem {
    std::string content;
    std::string hoverHint;
};

DEFINE_TYPE(ModList, ModListViewController);

void CreateListWithTitle(
    TransformWrapper parent, TransformWrapper titleParent, float columnWidth, std::string title, std::vector<ListItem> const& content
) {
    VerticalLayoutGroup* layout = CreateVerticalLayoutGroup(parent);
    layout->set_spacing(0.5);
    layout->set_childAlignment(UnityEngine::TextAnchor::UpperLeft);
    layout->set_childForceExpandHeight(false);
    layout->set_childControlHeight(true);

    // Create a layout for displaying the title.
    VerticalLayoutGroup* titleLayout = CreateVerticalLayoutGroup(titleParent);
    titleLayout->set_childForceExpandHeight(false);
    titleLayout->set_childControlHeight(true);
    titleLayout->GetComponent<LayoutElement*>()->set_minWidth(columnWidth);  // Make sure the list has a set width.
    titleLayout->GetComponent<LayoutElement*>()->set_preferredWidth(columnWidth);

    // Create the title text
    auto titleText = CreateText(titleLayout->get_rectTransform(), title);
    titleText->set_alignment(TMPro::TextAlignmentOptions::BottomLeft);
    titleText->set_overflowMode(TMPro::TextOverflowModes::Ellipsis);

    // Create a layout for the list itself
    VerticalLayoutGroup* listLayout = CreateVerticalLayoutGroup(layout->get_rectTransform());
    listLayout->GetComponent<LayoutElement*>()->set_minWidth(columnWidth);  // Make sure the list has a set width.
    listLayout->GetComponent<LayoutElement*>()->set_preferredWidth(columnWidth);

    // Add some padding so that the messages aren't totally squished
    titleLayout->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 0, 0));
    listLayout->set_padding(UnityEngine::RectOffset::New_ctor(1, 1, 0, 2));

    // Make sure the list items are in the top left
    listLayout->set_childAlignment(UnityEngine::TextAnchor::UpperLeft);
    listLayout->set_childForceExpandHeight(false);
    listLayout->set_childControlHeight(true);

    // Create a line of text for each in the list
    for (auto const& element : content) {
        TMPro::TextMeshProUGUI* text = CreateText(listLayout->get_rectTransform(), element.content);

        text->GetComponent<LayoutElement*>()->set_preferredWidth(columnWidth);
        text->set_overflowMode(TMPro::TextOverflowModes::Ellipsis);

        // Add a hover hint if there is one
        if (!element.hoverHint.empty()) {
            AddHoverHint(text->get_gameObject(), element.hoverHint);
        }
        text->set_fontSize(2.3f);
    }
}

void ModListViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!(firstActivation && addedToHierarchy)) {
        return;
    }

    // Create the main vertical layout for the mod list
    auto mainVerticalLayout = CreateVerticalLayoutGroup(get_rectTransform());
    mainVerticalLayout->set_childAlignment(TextAnchor::UpperLeft);
    mainVerticalLayout->set_childForceExpandHeight(false);
    mainVerticalLayout->set_childForceExpandWidth(false);
    mainVerticalLayout->set_childControlHeight(false);
    mainVerticalLayout->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 0, 0));

    // Create the horizontal layout for the titles
    auto titleHorizontalLayout = CreateHorizontalLayoutGroup(mainVerticalLayout);
    titleHorizontalLayout->set_childForceExpandHeight(false);
    titleHorizontalLayout->set_childForceExpandWidth(false);
    titleHorizontalLayout->set_childControlWidth(false);
    titleHorizontalLayout->set_childControlHeight(true);
    titleHorizontalLayout->set_childAlignment(TextAnchor::UpperLeft);
    titleHorizontalLayout->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 0, -1));
    titleHorizontalLayout->GetComponent<LayoutElement*>()->set_preferredWidth(160.0f);

    // Create a separator between the titles and the lists
    auto separatorHorizontalLayout = CreateHorizontalLayoutGroup(mainVerticalLayout);
    separatorHorizontalLayout->GetComponent<LayoutElement*>()->set_preferredWidth(164.0f);
    separatorHorizontalLayout->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 0, 0));

    auto separator = CreateText(
        separatorHorizontalLayout->get_rectTransform(),
        "________________________________________________________________________________"
        "________________________________________________________________________________"
        "________________________________________________________________________________"
        "________________________________________________________________________________"
        "________________________________________________________________________________"
        "________________________________________________________________________________"
        "________________________________________________________________________________"
        "________________________________________________________________________________"
    );
    separator->set_fontSize(1.0);
    separator->set_overflowMode(TMPro::TextOverflowModes::Truncate);
    separator->set_enableWordWrapping(false);

    // Create the continaer layout for the scroll view
    auto scrollLayout = CreateHorizontalLayoutGroup(mainVerticalLayout);
    scrollLayout->set_childForceExpandHeight(true);
    scrollLayout->set_childForceExpandWidth(true);
    scrollLayout->GetComponent<LayoutElement*>()->set_preferredWidth(164.0f);
    scrollLayout->GetComponent<LayoutElement*>()->set_preferredHeight(70.0f);

    // Allow the lists to be scrolled
    UnityEngine::GameObject* scrollView = CreateScrollableSettingsContainer(scrollLayout);

    // Create the main layout for the lists
    HorizontalLayoutGroup* mainLayout = CreateHorizontalLayoutGroup(scrollView->get_transform());
    mainLayout->set_childAlignment(UnityEngine::TextAnchor::UpperLeft);  // The lists should Left aligned
    mainLayout->set_childForceExpandHeight(false);
    mainLayout->set_childControlHeight(true);

    // Check to see which libraries loaded/failed to load
    Logger.info("Checking library load info.");

    // Find the path with the correct application ID
    LibraryLoadInfo& libraryLoadInfo = GetModloaderLibsLoadInfo();
    std::vector<ListItem> librariesList;

    for (std::pair<std::string, std::optional<std::string>> libraryLoadPair : libraryLoadInfo) {
        if (libraryLoadPair.second.has_value()) {
            // If there was an error loading the library, display it in red
            Logger.debug("Adding failed library {}", libraryLoadPair.first.c_str());
            ListItem item;
            item.content = "<color=red>" + libraryLoadPair.first;
            item.hoverHint = *libraryLoadPair.second;  // Allow you to hover over the mod to see the fail reason
            librariesList.push_back(item);
        } else {
            // Otherwise, make the library name green
            Logger.debug("Adding successful library {}", libraryLoadPair.first.c_str());
            ListItem item;
            item.content = "<color=green>" + libraryLoadPair.first;
            librariesList.push_back(item);
        }
    }

    Logger.info("Adding loaded mods . . .");
    // Find the list of all loaded mods
    std::vector<ListItem> loadedMods;
    auto modloaderLoadedMods = modloader_get_loaded();
    for (int i = 0; i < modloaderLoadedMods.size; i++) {
        CModResult const& mod = modloaderLoadedMods.array[i];

        std::string libsPath = fmt::format("{}/mods", modloader_get_files_dir());
        if (!std::string(mod.path).starts_with(libsPath)) {
            continue;
        }

        Logger.info("Adding mod {}", mod.info.id);
        ListItem item;
        std::string id = strlen(mod.info.id) == 0 ? mod.path : mod.info.id;
        if (id.find("/") != std::string::npos) {
            id = id.substr(id.find_last_of("/") + 1);
        }
        item.content = fmt::format("<color=green>{}</color><color=white> v{}", id, mod.info.version);
        loadedMods.push_back(item);
    }

    std::vector<ListItem> loadedEarlyMods;
    auto modloaderLoadedEarlyMods = modloader_get_loaded();
    for (int i = 0; i < modloaderLoadedEarlyMods.size; i++) {
        CModResult const& mod = modloaderLoadedEarlyMods.array[i];

        std::string libsPath = fmt::format("{}/early_mods", modloader_get_files_dir());
        if (!std::string(mod.path).starts_with(libsPath)) {
            continue;
        }

        Logger.info("Adding mod {}", mod.info.id);
        ListItem item;
        std::string id = strlen(mod.info.id) == 0 ? mod.path : mod.info.id;
        if (id.find("/") != std::string::npos) {
            id = id.substr(id.find_last_of("/") + 1);
        }
        item.content = fmt::format("<color=green>{}</color><color=white> v{}", id, mod.info.version);
        loadedEarlyMods.push_back(item);
    }

    // Find the info about why the libraries in the mods or early_mods directory loaded/didn't load
    // Make sure to find the mods path with the correct application ID
    LibraryLoadInfo& modsLoadInfo = GetModsLoadInfo();
    // LibraryLoadInfo& earlyModsLoadInfo = GetEarlyModsLoadInfo();
    // modsLoadInfo.insert(earlyModsLoadInfo.begin(), earlyModsLoadInfo.end());

    std::vector<ListItem> failedMods;
    Logger.info("Checking for failed mods . . .");
    for (std::pair<std::string, std::optional<std::string>> modLoadPair : modsLoadInfo) {
        // If there was an error loading the library, add it to the list in red
        if (modLoadPair.second.has_value()) {
            Logger.debug("Adding failed mod {}", modLoadPair.first.c_str());
            ListItem item;
            item.content = fmt::format("<color=red>{}", modLoadPair.first.c_str());
            item.hoverHint = *modLoadPair.second;  // Allow you to hover over the mod to see the fail reason
            failedMods.push_back(item);
        }
    }

    LibraryLoadInfo& earlyModsLoadInfo = GetEarlyModsLoadInfo();

    std::vector<ListItem> failedEarlyMods;
    Logger.info("Checking for failed mods . . .");
    for (std::pair<std::string, std::optional<std::string>> modLoadPair : earlyModsLoadInfo) {
        // If there was an error loading the library, add it to the list in red
        if (modLoadPair.second.has_value()) {
            Logger.debug("Adding failed mod {}", modLoadPair.first.c_str());
            ListItem item;
            item.content = fmt::format("<color=red>{}", modLoadPair.first.c_str());
            item.hoverHint = *modLoadPair.second;  // Allow you to hover over the mod to see the fail reason
            failedEarlyMods.push_back(item);
        }
    }

    // Create lists for each group
    CreateListWithTitle(mainLayout, titleHorizontalLayout, 31.5, "Loaded Early Mods", loadedEarlyMods);
    CreateListWithTitle(mainLayout, titleHorizontalLayout, 31.5, "Loaded Mods", loadedMods);
    CreateListWithTitle(mainLayout, titleHorizontalLayout, 31.5, "Failed Early Mods", failedEarlyMods);
    CreateListWithTitle(mainLayout, titleHorizontalLayout, 31.5, "Failed Mods", failedMods);
    CreateListWithTitle(mainLayout, titleHorizontalLayout, 31.5, "Libraries", librariesList);
}

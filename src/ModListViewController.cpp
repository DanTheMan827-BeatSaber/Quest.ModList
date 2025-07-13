#include "ModListViewController.hpp"

#include "assets.hpp"
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

#include "HMUI/Touchable.hpp"

// BSML
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
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
    // layout->name = title;
    layout->set_spacing(0.5);
    layout->set_childAlignment(UnityEngine::TextAnchor::UpperLeft);
    layout->set_childForceExpandHeight(false);
    layout->set_childControlHeight(true);

    // Create a layout for displaying the title.
    VerticalLayoutGroup* titleLayout = CreateVerticalLayoutGroup(titleParent);
    titleLayout->name = "TitleWrapper";
    titleLayout->set_childForceExpandHeight(false);
    titleLayout->set_childControlHeight(true);
    titleLayout->GetComponent<LayoutElement*>()->set_minWidth(columnWidth);  // Make sure the list has a set width.
    titleLayout->GetComponent<LayoutElement*>()->set_preferredWidth(columnWidth);
    titleLayout->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 0, 0));

    // Create the title text
    auto titleText = CreateText(titleLayout->get_rectTransform(), title);
    titleText->name = "TitleText";
    titleText->set_alignment(TMPro::TextAlignmentOptions::BottomLeft);
    titleText->set_overflowMode(TMPro::TextOverflowModes::Ellipsis);

    // Create a layout for the list itself
    VerticalLayoutGroup* listLayout = CreateVerticalLayoutGroup(layout->get_rectTransform());
    listLayout->name = "ModsVerticalLayout";
    listLayout->GetComponent<LayoutElement*>()->set_minWidth(columnWidth);  // Make sure the list has a set width.
    listLayout->GetComponent<LayoutElement*>()->set_preferredWidth(columnWidth);
    listLayout->set_padding(UnityEngine::RectOffset::New_ctor(1, 1, 1, 1));
    listLayout->set_childAlignment(UnityEngine::TextAnchor::UpperLeft);
    listLayout->set_childForceExpandHeight(false);
    listLayout->set_childControlHeight(true);

    // Create a line of text for each in the list
    for (auto const& element : content) {
        TMPro::TextMeshProUGUI* text = CreateText(listLayout->get_rectTransform(), element.content);
        text->name = "ModText";
        text->GetComponent<LayoutElement*>()->set_preferredWidth(columnWidth);
        text->set_overflowMode(TMPro::TextOverflowModes::Ellipsis);

        // Add a hover hint if there is one
        if (!element.hoverHint.empty()) {
            AddHoverHint(text->get_gameObject(), element.hoverHint);
        }
        text->set_fontSize(2.3f);
    }
}

/// @brief Creates a canvas with specified size and position, and attaches it to the given parent.
/// @param parent The parent transform to attach the canvas to.
/// @param sizeDelta The size of the canvas.
/// @param anchoredPosition The anchored position of the canvas.
/// @return A pointer to the created RectTransform.
RectTransform* createCanvas(TransformWrapper parent, Vector2 sizeDelta, Vector2 anchoredPosition) {
    auto canvas = BSML::Lite::CreateCanvas();
    auto rectTransform = canvas->GetComponent<UnityEngine::RectTransform*>();

    rectTransform->SetParent(parent, false);
    rectTransform->localScale = {1, 1, 1};
    rectTransform->sizeDelta = sizeDelta;
    rectTransform->anchoredPosition = anchoredPosition;
    rectTransform->GetComponent<UnityEngine::Canvas*>()->set_overrideSorting(true);

    return rectTransform;
}

HMUI::ImageView* drawLine(TransformWrapper parent, Vector2 const start, Vector2 const end, float thickness = 0.3f) {
    static auto whitePixel = BSML::Utilities::ImageResources::GetWhitePixel();

    Vector2 start2 = {start.x, -start.y};
    Vector2 end2 = {end.x, -end.y};

    // Manually compute the difference vector.
    Vector2 diff;
    diff.x = end2.x - start2.x;
    diff.y = end2.y - start2.y;

    // Compute the distance between the two points.
    float distance = std::hypot(diff.x, diff.y);

    auto pixelImage = BSML::Lite::CreateImage(parent, whitePixel, {0, 0}, {0, 0});
    auto pixelRect = pixelImage->rectTransform;

    pixelRect->pivot = {0, 0};
    pixelRect->anchorMin = {0, 1};
    pixelRect->anchorMax = {0, 1};
    pixelRect->anchoredPosition = start2;

    // Set the size: width equals the distance, height equals the thickness.
    pixelRect->sizeDelta = {distance, thickness};

    // Compute the angle (in degrees) between the start and end points.
    // Note: std::atan2 returns radians.
    float angle = std::atan2(diff.y, diff.x) * 180.0f / 3.14159265f;

    // Rotate the element so it aligns with the line direction.
    pixelRect->localEulerAngles = {0.0f, 0.0f, angle};

    return pixelImage;
}

void ModListViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!(firstActivation && addedToHierarchy)) {
        return;
    }

    // Create the main vertical layout for the mod list
    auto mainStack = rectTransform;

    // Create the horizontal layout for the titles
    auto titleHorizontalLayout = CreateHorizontalLayoutGroup(createCanvas(mainStack, {164, 5}, {2.25, 37}));
    titleHorizontalLayout->name = "TitleHorizontalLayout";
    titleHorizontalLayout->set_childForceExpandHeight(false);
    titleHorizontalLayout->set_childForceExpandWidth(false);
    titleHorizontalLayout->set_childControlWidth(false);
    titleHorizontalLayout->set_childControlHeight(true);
    titleHorizontalLayout->set_childAlignment(TextAnchor::UpperLeft);
    titleHorizontalLayout->GetComponent<LayoutElement*>()->set_preferredWidth(160.0f);
    titleHorizontalLayout->GetComponent<RectTransform*>()->set_anchoredPosition({3.5, 0});

    // Create the continaer layout for the scroll view
    auto scrollLayout = CreateHorizontalLayoutGroup(createCanvas(mainStack, {164, 71.85}, {2, 1.2}));
    scrollLayout->name = "ScrollWrapper";
    scrollLayout->set_childForceExpandHeight(true);
    scrollLayout->set_childForceExpandWidth(true);
    scrollLayout->GetComponent<LayoutElement*>()->set_preferredWidth(164.0f);
    scrollLayout->GetComponent<LayoutElement*>()->set_preferredHeight(70.0f);
    scrollLayout->GetComponent<RectTransform*>()->set_anchoredPosition({3.5, 0});

    // Allow the lists to be scrolled
    auto scrollView = CreateScrollableSettingsContainer(scrollLayout);
    scrollView->name = "ScrollView";
    scrollView->transform->parent->parent->GetComponent<UnityEngine::RectTransform*>()->set_sizeDelta({-8, -4});

    // Create a canvas for the background lines
    auto backgroundCanvas = createCanvas(mainStack, {159.5, 74.55}, {0, 0});
    backgroundCanvas->anchoredPosition = {1, 3.14};

    // Draw our background image
    // auto background = CreateImage(backgroundCanvas, PNG_SPRITE(IncludedAssets::ModList::frame_png));
    // background->get_rectTransform()->sizeDelta = {159.5, 74.55};
    // background->get_rectTransform()->anchoredPosition = {-0.15, 0.15};

    // Draw a box around the perimeter of the canvas
    drawLine(backgroundCanvas, {0, 0.15}, {157.5, 0.15}, 0.3f)->name = "Top";
    drawLine(backgroundCanvas, {0, 0}, {0, 72.55}, 0.3f)->name = "Left";
    drawLine(backgroundCanvas, {157.5 - 0.3, 0}, {157.5 - 0.3, 72.55}, 0.3f)->name = "Right";
    drawLine(backgroundCanvas, {0, 72.55}, {157.5, 72.55}, 0.3f)->name = "Bottom";
    drawLine(backgroundCanvas, {0, 6.27}, {157.5, 6.27}, 0.3f)->name = "HorizontalDivider";

    // Draw our divider lines
    for (auto i = 1; i <= 4; i++) {
        drawLine(backgroundCanvas, {(31.5f * i), 0}, {(31.5f * i), 72.55}, 0.3f)->name = fmt::format("VerticalDivider{}", i);
    }

    // Create the main layout for the lists
    auto mainLayout = CreateHorizontalLayoutGroup(scrollView);
    mainLayout->name = "HorizontalModColumns";
    mainLayout->set_childAlignment(UnityEngine::TextAnchor::UpperLeft);  // The lists should Left aligned
    mainLayout->set_childForceExpandHeight(false);
    mainLayout->set_childControlHeight(true);

    // Check to see which libraries loaded/failed to load
    Logger.info("Checking library load info.");

    // Find the path with the correct application ID
    LibraryLoadInfo& libraryLoadInfo = GetModloaderLibsLoadInfo();
    std::vector<ListItem> librariesList;

    // Add the libraries to the list
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

    // Populate the list of all loaded mods
    Logger.info("Adding loaded mods . . .");
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
        item.content = fmt::format("<color=green>{}</color><color=white> v{}", id, strlen(mod.info.version) == 0 ? "0.0.0" : mod.info.version);
        loadedMods.push_back(item);
    }

    // Populate the list of all loaded early mods
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
        item.content = fmt::format("<color=green>{}</color><color=white> v{}", id, strlen(mod.info.version) == 0 ? "0.0.0" : mod.info.version);
        loadedEarlyMods.push_back(item);
    }

    // Find the info about why the libraries in the mods or early_mods directory loaded/didn't load
    // Make sure to find the mods path with the correct application ID
    LibraryLoadInfo& modsLoadInfo = GetModsLoadInfo();
    // LibraryLoadInfo& earlyModsLoadInfo = GetEarlyModsLoadInfo();
    // modsLoadInfo.insert(earlyModsLoadInfo.begin(), earlyModsLoadInfo.end());

    // Populate the list of all failed mods
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

    // Get the early mods load info
    LibraryLoadInfo& earlyModsLoadInfo = GetEarlyModsLoadInfo();

    // Populate the list of all failed early mods
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

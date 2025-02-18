#include "config.hpp"

#include "bsml/shared/BSML-Lite/Creation/Layout.hpp"
#include "config-utils/shared/config-utils.hpp"
#include "HMUI/ViewController.hpp"
#include "UnityEngine/GameObject.hpp"

void ConfigViewDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    using namespace UnityEngine;

    if (firstActivation) {
        auto container = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());

        AddConfigValueToggle(container, getConfig().showFailedOnStart);
    }
}

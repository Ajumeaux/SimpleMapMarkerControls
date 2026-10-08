/*
* Simple Map Marker Controls
* Copyright (C) 2026 Ajumeaux
*
* SPDX-License-Identifier: GPL-3.0-or-later
*
* This program is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License, version 3 or later.
*
* This program is distributed WITHOUT ANY WARRANTY.
* See the LICENSE file for details.
*/

#ifndef LOCALMAPUPGRADECOMPAT_HPP
#define LOCALMAPUPGRADECOMPAT_HPP

#include <RE/Skyrim.h>
#include <REL/Relocation.h>

#include "PlayerMarker.hpp"

namespace logger = SKSE::log;

class LocalMapUpgradeCompat
{
public:
    static void Install()
    {
        REL::Relocation<std::uintptr_t> vtable{ RE::VTABLE_LocalMapMenu__InputHandler[0] };

        _originalProcessButton = vtable.write_vfunc(5, ProcessButton);

        logger::info("Installed Local Map Upgrade compatibility hook.");
    }

private:
    using ProcessButton_t = bool (*)(RE::LocalMapMenu::InputHandler*, RE::ButtonEvent*);

    inline static REL::Relocation<ProcessButton_t> _originalProcessButton;

    static bool ProcessButton(RE::LocalMapMenu::InputHandler* handler, RE::ButtonEvent* event)
    {
        if (!handler || !event) {
            return _originalProcessButton(handler, event);
        }

        const bool isLeftClick = event->device.get() == RE::INPUT_DEVICE::kMouse && event->GetIDCode() == 0;

        if (!isLeftClick) {
            return _originalProcessButton(handler, event);
        }

        auto* localMap = handler->localMapMenu;

        if (!localMap) {
            return _originalProcessButton(handler, event);
        }

        auto& runtime = localMap->GetRuntimeData();

        if (!runtime.showingMap || !event->IsDown()) {
            return _originalProcessButton(handler, event);
        }

        const auto selectedIndex = runtime.selectedMarker;
        bool clickedPlayerMarker = false;

        if (selectedIndex >= 0 && static_cast<std::size_t>(selectedIndex) < localMap->mapMarkers.size()) {
            clickedPlayerMarker = PlayerMarker::IsPlayerSetMarker(localMap->mapMarkers[selectedIndex]);
        }

        if (clickedPlayerMarker && PlayerMarker::Exists()) {
            if (!PlayerMarker::Remove()) {
                logger::warn("LocalMapUpgrade: failed to remove PlayerSetMarker");
                return _originalProcessButton(handler, event);
            }

            const bool result = _originalProcessButton(handler, event);

            if (PlayerMarker::Exists() && !PlayerMarker::Remove()) {
                logger::warn("LocalMapUpgrade: failed to remove temporary replacement PlayerSetMarker");
            }

            return result;
        }

        if (PlayerMarker::Exists()) {
            if (!PlayerMarker::Remove()) {
                logger::warn("LocalMapUpgrade: failed to remove PlayerSetMarker before move");
            }
        }

        return _originalProcessButton(handler, event);
    }
};

#endif // LOCALMAPUPGRADECOMPAT_HPP

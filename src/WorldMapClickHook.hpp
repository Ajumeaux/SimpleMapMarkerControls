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

#ifndef WORLDMAPCLICKHOOK_HPP
#define WORLDMAPCLICKHOOK_HPP

#include <RE/Skyrim.h>
#include <REL/Relocation.h>

#include <cstring>

#include "PlayerMarker.hpp"

namespace logger = SKSE::log;

class WorldMapClickHook
{
public:
    static void Install()
    {
        REL::Relocation<std::uintptr_t> vtable{ RE::VTABLE_MapMenu[0] };

        _originalAccept = vtable.write_vfunc(1, Accept);

        logger::info("Installed World Map ClickCallback hook.");
    }

private:
    using CallbackProcessor = RE::FxDelegateHandler::CallbackProcessor;
    using CallbackFn = RE::FxDelegateHandler::CallbackFn;
    using Accept_t = void (*)(RE::MapMenu*, CallbackProcessor*);

    inline static REL::Relocation<Accept_t> _originalAccept;
    inline static CallbackFn* _originalClickCallback = nullptr;

    static bool HasPlayerSetMarker(const RE::BSTArray<RE::MapMenuMarker>& markers)
    {
        for (const auto& marker : markers) {
            if (PlayerMarker::IsPlayerSetMarker(marker)) {
                return true;
            }
        }

        return false;
    }

    class ProcessorProxy final : public CallbackProcessor
    {
    public:
        explicit ProcessorProxy(CallbackProcessor* original) : _original(original)
        {}

        void Process(const RE::GString& methodName, CallbackFn* method) override
        {
            if (std::strcmp(methodName.c_str(), "ClickCallback") == 0) {
                _originalClickCallback = method;
                _original->Process(methodName, WorldMapClickHook::ClickCallback);
                return;
            }

            _original->Process(methodName, method);
        }

    private:
        CallbackProcessor* _original;
    };

    static void Accept(RE::MapMenu* mapMenu, CallbackProcessor* processor)
    {
        ProcessorProxy proxy{ processor };
        _originalAccept(mapMenu, &proxy);
    }

    static void HandleWorldMapClick(std::int32_t selectedIndex, const RE::BSTArray<RE::MapMenuMarker>& markers, const RE::FxDelegateArgs& args)
    {
        if (selectedIndex < 0) {
            if (HasPlayerSetMarker(markers)) {
                if (!PlayerMarker::Remove()) {
                    logger::warn("WorldMap: failed to remove existing PlayerSetMarker before move");
                    return;
                }
            }

            CallOriginal(args);
            return;
        }

        if (static_cast<std::size_t>(selectedIndex) >= markers.size()) {
            logger::warn("WorldMap: selectedMarker {} outside mapMarkers (size={})", selectedIndex, markers.size());
            CallOriginal(args);
            return;
        }

        const auto& marker = markers[selectedIndex];

        if (PlayerMarker::IsPlayerSetMarker(marker)) {
            PlayerMarker::Remove();
            return;
        }

        CallOriginal(args);
    }

    static void ClickCallback(const RE::FxDelegateArgs& args)
    {
        auto* ui = RE::UI::GetSingleton();

        if (!ui) {
            CallOriginal(args);
            return;
        }

        auto menu = ui->GetMenu<RE::MapMenu>();

        if (!menu) {
            CallOriginal(args);
            return;
        }

        auto* runtime = menu->GetRuntimeData();

        if (!runtime) {
            CallOriginal(args);
            return;
        }

        if (runtime->localMapMenu.GetRuntimeData().showingMap) {
            CallOriginal(args);
            return;
        }

        const auto* worldRuntime = menu->GetRuntimeData2();

        if (!worldRuntime) {
            CallOriginal(args);
            return;
        }

        HandleWorldMapClick(worldRuntime->selectedMarker, worldRuntime->mapMarkers, args);
    }

    static void CallOriginal(const RE::FxDelegateArgs& args)
    {
        if (_originalClickCallback) {
            _originalClickCallback(args);
        }
    }
};

#endif // WORLDMAPCLICKHOOK_HPP

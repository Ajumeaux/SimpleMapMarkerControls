#ifndef MAPCLICKHOOK_HPP
#define MAPCLICKHOOK_HPP

#include <RE/Skyrim.h>
#include <REL/Relocation.h>

#include <cstring>

#include "PlayerMarker.hpp"

namespace logger = SKSE::log;

class MapClickHook
{
public:
    static void Install()
    {
        REL::Relocation<std::uintptr_t> vtable{ RE::VTABLE_MapMenu[0] };

        // FxDelegateHandler::Accept is virtual slot 01.
        _originalAccept = vtable.write_vfunc(1, Accept);

        logger::info("Installed MapMenu ClickCallback hook.");
    }

private:
    using CallbackProcessor = RE::FxDelegateHandler::CallbackProcessor;
    using CallbackFn = RE::FxDelegateHandler::CallbackFn;
    using Accept_t = void (*)(RE::MapMenu*, CallbackProcessor*);

    inline static REL::Relocation<Accept_t> _originalAccept;
    inline static CallbackFn* _originalClickCallback = nullptr;

    static constexpr std::uint32_t kPlayerSetMarkerType = 2;

    static bool IsPlayerSetMarker(const RE::MapMenuMarker& marker)
    {
        return marker.type == kPlayerSetMarkerType && marker.fullName == nullptr && marker.form == nullptr;
    }

    static bool HasPlayerSetMarker(const RE::BSTArray<RE::MapMenuMarker>& markers)
    {
        for (const auto& marker : markers) {
            if (IsPlayerSetMarker(marker)) {
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
            logger::info("MapMenu callback registered: {}", methodName.c_str());

            if (std::strcmp(methodName.c_str(), "ClickCallback") == 0) {
                _originalClickCallback = method;

                logger::info("Intercepted MapMenu ClickCallback registration.");

                _original->Process(methodName, MapClickHook::ClickCallback);
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

    static void HandleMapClick(std::int32_t selectedIndex, const RE::BSTArray<RE::MapMenuMarker>& markers, const RE::FxDelegateArgs& args, const char* mapName)
    {
        if (selectedIndex < 0) {
            if (HasPlayerSetMarker(markers)) {
                logger::info("{}: moving PlayerSetMarker", mapName);

                if (!PlayerMarker::Remove()) {
                    logger::warn("{}: failed to remove existing PlayerSetMarker before move", mapName);
                    return;
                }
            } else {
                logger::info("{}: placing PlayerSetMarker", mapName);
            }

            // Let vanilla place the marker at the current cursor position.
            CallOriginal(args);
            return;
        }

        if (static_cast<std::size_t>(selectedIndex) >= markers.size()) {
            logger::warn("{}: selectedMarker {} outside mapMarkers (size={})", mapName, selectedIndex, markers.size());

            CallOriginal(args);
            return;
        }

        const auto& marker = markers[selectedIndex];

        if (IsPlayerSetMarker(marker)) {
            logger::info("{}: removing PlayerSetMarker (index={})", mapName, selectedIndex);

            PlayerMarker::Remove();
            return;
        }

        logger::info("{}: regular marker (index={}, type={})", mapName, selectedIndex, marker.type);

        // Preserve vanilla behavior for normal map markers.
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
        const auto* worldRuntime = menu->GetRuntimeData2();

        if (!runtime) {
            CallOriginal(args);
            return;
        }

        auto& localMap = runtime->localMapMenu;
        auto& localRuntime = localMap.GetRuntimeData();

        logger::info(
            "Map state: local.showingMap={}, local.selectedMarker={}, local.markers={}, world.selectedMarker={}, world.markers={}",
            localRuntime.showingMap,
            localRuntime.selectedMarker,
            localMap.mapMarkers.size(),
            worldRuntime->selectedMarker,
            worldRuntime->mapMarkers.size()
        );

        if (localRuntime.showingMap) {
            HandleMapClick(localRuntime.selectedMarker, localMap.mapMarkers, args, "LocalMap");
            return;
        }

        if (!worldRuntime) {
            CallOriginal(args);
            return;
        }

        HandleMapClick(worldRuntime->selectedMarker, worldRuntime->mapMarkers, args, "WorldMap");
    }

    static void CallOriginal(const RE::FxDelegateArgs& args)
    {
        if (_originalClickCallback) {
            _originalClickCallback(args);
        }
    }
};

#endif // MAPCLICKHOOK_HPP
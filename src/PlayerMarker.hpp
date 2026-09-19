#ifndef PLAYERMARKER_HPP
#define PLAYERMARKER_HPP

#include <RE/Skyrim.h>
#include <REL/Relocation.h>

namespace PlayerMarker
{
    inline constexpr std::uint32_t kPlayerSetMarkerType = 2;

    inline bool IsPlayerSetMarker(const RE::MapMenuMarker& marker)
    {
        return marker.type == kPlayerSetMarkerType && marker.fullName == nullptr && marker.form == nullptr;
    }

    inline bool Exists()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        return player && static_cast<bool>(player->GetInfoRuntimeData().playerMapMarker);
    }

    namespace detail
    {
        inline constexpr REL::ID kClearPlayerMarkerID{ 40536 };
        inline constexpr REL::ID kPlayerStateID{ 403521 };
        inline constexpr REL::ID kCurrentPositionID{ 406663 };
        inline constexpr REL::ID kDefaultPositionID{ 410468 };

        using ClearPlayerMarker_t = void (*)(void*);

        inline REL::Relocation<ClearPlayerMarker_t> ClearPlayerMarker{ kClearPlayerMarkerID };
        inline REL::Relocation<void**> PlayerState{ kPlayerStateID };
        inline REL::Relocation<RE::NiPoint3*> CurrentPosition{ kCurrentPositionID };
        inline REL::Relocation<RE::NiPoint3*> DefaultPosition{ kDefaultPositionID };
    }

    inline bool Remove()
    {
        void* state = *detail::PlayerState;

        if (!state) {
            SKSE::log::warn("PlayerMarker::Remove: player state is null");
            return false;
        }

        detail::ClearPlayerMarker(state);
        *detail::CurrentPosition = *detail::DefaultPosition;

        return true;
    }
}

#endif // PLAYERMARKER_HPP

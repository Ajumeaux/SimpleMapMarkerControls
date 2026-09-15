#ifndef MENUEVENTHANDLER_H
#define MENUEVENTHANDLER_H

#include <RE/Skyrim.h>

namespace logger = SKSE::log;

class MenuEventHandler final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    static MenuEventHandler* GetSingleton()
    {
        static MenuEventHandler instance;
        return &instance;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
    {
        if (!event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (event->menuName == RE::MapMenu::MENU_NAME) {
            logger::info("MapMenu {}", event->opening ? "opened" : "closed");
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

#endif // MENUEVENTHANDLER_H
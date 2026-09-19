#include <SKSE/SKSE.h>

#include <spdlog/sinks/basic_file_sink.h>

#include "LocalMapUpgradeCompat.hpp"
#include "WorldMapClickHook.hpp"

namespace logger = SKSE::log;

static void SetupLogging()
{
    auto logDirectory = logger::log_directory();

    if (!logDirectory) {
        SKSE::stl::report_and_fail("Failed to find SKSE log directory.");
    }

    const auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();

    *logDirectory /= std::format("{}.log", pluginName);

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logDirectory->string(), true);
    auto log = std::make_shared<spdlog::logger>("global", std::move(sink));

    log->set_level(spdlog::level::trace);
    log->flush_on(spdlog::level::trace);

    spdlog::set_default_logger(std::move(log));
}

static void OnSKSEMessage(SKSE::MessagingInterface::Message* message)
{
    if (!message || message->type != SKSE::MessagingInterface::kPostLoad) {
        return;
    }

    if (GetModuleHandleW(L"LocalMapUpgrade.dll")) {
        LocalMapUpgradeCompat::Install();
        logger::info("Local Map Upgrade detected. Compatibility enabled.");
    } else {
        logger::info("Local Map Upgrade not detected. Local Map compatibility disabled.");
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);

    SetupLogging();

    logger::info("Simple Map Marker Controls loaded successfully.");

    WorldMapClickHook::Install();
    SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessage);

    return true;
}

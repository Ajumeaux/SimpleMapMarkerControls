#include <SKSE/SKSE.h>

#include <spdlog/sinks/basic_file_sink.h>

#include "MapClickHook.hpp"
#include "MenuEventHandler.hpp"

namespace logger = SKSE::log;

static void setup_logging()
{
    auto log_directory = logger::log_directory();

    if (!log_directory) {
        SKSE::stl::report_and_fail("Failed to find SKSE log directory.");
    }

    const auto plugin_name = SKSE::PluginDeclaration::GetSingleton()->GetName();

    *log_directory /= std::format("{}.log", plugin_name);

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_directory->string(), true);
    auto log = std::make_shared<spdlog::logger>("global", std::move(sink));

    log->set_level(spdlog::level::trace);
    log->flush_on(spdlog::level::trace);

    spdlog::set_default_logger(std::move(log));
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);

    setup_logging();

    logger::info("Simple Map Marker Controls loaded successfully.");

    if (auto* ui = RE::UI::GetSingleton()) {
        ui->AddEventSink(MenuEventHandler::GetSingleton());
        logger::info("Registered MenuOpenCloseEvent listener.");
    } else {
        logger::error("UI singleton is null.");
    }

    MapClickHook::Install();

    return true;
}
#define DLLEXPORT __declspec(dllexport)

#include "InterfaceQuickLootIE.h"
#include "Config.h"
#include "UI.h"

void InitializeLog([[maybe_unused]] spdlog::level::level_enum a_level = spdlog::level::trace)
{
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log"sv, Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	const auto level = a_level;

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(spdlog::level::trace);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		if (!QuickLootDD::InterfaceQuickLootIE::Init()) {
			ERROR("Error in QuickLootIE integration");
		}
		if (!QuickLootDD::InterfaceDeviouslyEnchantedChests::Init()) {
			ERROR("Error in DEC integration");
		}
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		QuickLootDD::InterfaceDeviouslyEnchantedChests::LoadForms();
		QuickLootDD::UI::OnDataLoaded();
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
		[[fallthrough]];
	case SKSE::MessagingInterface::kNewGame:
		DEBUG("DeviouslyEnchantedChests Reset");
		QuickLootDD::InterfaceDeviouslyEnchantedChests::Reset();
		break;
	}
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	LOG("Loaded plugin {} {}", Plugin::NAME, Plugin::VERSION.string());
	SKSE::Init(a_skse);
	QuickLootDD::Config::readIniConfig();

    auto message = SKSE::GetMessagingInterface();
	if (!message->RegisterListener(MessageHandler)) {
		return false;
	}

    if (QuickLootDD::Config::visualiseChance) {
		if (!QuickLootDD::UI::Install()) {
			ERROR("Failed to install UI Renderer"sv);
		}
	}

	return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary(true);
	v.HasNoStructUse();
	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}
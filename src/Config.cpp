#include "Config.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace QuickLootDD
{

	static std::vector<std::string> ini_string_to_array(std::string value)
	{
		std::vector<std::string> loc_res;
		std::string a_sep = ",";
		try {
			boost::split(loc_res, value, boost::is_any_of(a_sep));
		} catch (...) {
			ERROR("Can't get config array from '{}' - Returning empty array", value)
			loc_res = std::vector<std::string>();
		}

		//remove additional spaces
		for (auto&& it : loc_res) {
			const auto loc_first = it.find_first_not_of(' ');
			const auto loc_last = it.find_last_not_of(' ');
			it = it.substr(loc_first, loc_last - loc_first + 1);
		}

		return loc_res;
	}

	void Config::readIniConfig()
	{
		auto iniConfig = boost::property_tree::ptree();
		try {
			boost::property_tree::ini_parser::read_ini("Data\\skse\\plugins\\QuickLootIEDD.ini", iniConfig);

			useDECContainerList = iniConfig.get<bool>("DEC.useDECContainerList", true);
			allowDeadBody = iniConfig.get<bool>("DEC.allowDeadBody", false);
			allowActorTypeNPC = iniConfig.get<bool>("DEC.allowActorTypeNPC", false);
			allowActorTypeCreature = iniConfig.get<bool>("DEC.allowActorTypeCreature", false);
			allowActorTypeAnimal = iniConfig.get<bool>("DEC.allowActorTypeAnimal", false);
			allowActorTypeDragon = iniConfig.get<bool>("DEC.allowActorTypeDragon", false);
			allowOther = iniConfig.get<bool>("DEC.allowOther", false);

			baseChanceMin = iniConfig.get<double>("DEC.baseChanceMin", 0.01);
            if (baseChanceMin < 0) {
				WARN("readIniConfig: incorrect DEC.baseChanceMin, adjusted to `0.0`");
				baseChanceMin = 0.0;
			}
			if (baseChanceMin > 1) {
				WARN("readIniConfig: incorrect DEC.baseChanceMin, adjusted to `1.0`");
				baseChanceMin = 1.0;
			}

			baseChanceMax = iniConfig.get<double>("DEC.baseChanceMax", 0.1);
			if (baseChanceMax < baseChanceMin) {
				WARN("readIniConfig: incorrect DEC.baseChanceMax, adjusted to `DEC.baseChanceMin`");
				baseChanceMax = baseChanceMin;
			}

			chanceLimit = iniConfig.get<double>("DEC.chanceLimit", 0.5);
			if (chanceLimit < 0) {
				WARN("readIniConfig: incorrect DEC.chanceLimit, adjusted to `0.0`");
				chanceLimit = 0.0;
			}
			if (baseChanceMin > 1) {
				WARN("readIniConfig: incorrect DEC.chanceLimit, adjusted to `1.0`");
				chanceLimit = 1.0;
			}

			containerTriggerCooldown = iniConfig.get<int>("DEC.containerTriggerCooldown", 60);

			increaseChanceMultiplierMin = iniConfig.get<double>("DEC.increaseChanceMultiplierMin", 1.01);
			if (increaseChanceMultiplierMin < 1) {
				WARN("readIniConfig: incorrect DEC.increaseChanceMultiplierMin, adjusted to `1.0`");
				increaseChanceMultiplierMin = 1.0;
			}

			increaseChanceMultiplierMax = iniConfig.get<double>("DEC.increaseChanceMultiplierMax", 1.1);
			if (increaseChanceMultiplierMax < increaseChanceMultiplierMin) {
				WARN("readIniConfig: incorrect DEC.increaseChanceMultiplierMax, adjusted to `DEC.increaseChanceMultiplierMin`");
				increaseChanceMultiplierMax = increaseChanceMultiplierMin;
			}

			containerChanceCooldown = iniConfig.get<int>("DEC.containerChanceCooldown", 3600);
			resetChanceOnLastItem = iniConfig.get<bool>("DEC.resetChanceOnLastItem", true);

			containerLimit = iniConfig.get<int>("DEC.containerLimit", 100);

            SafeLocations = ini_string_to_array(iniConfig.get<std::string>("DEC.SafeLocations", ""));

			CityLocations = ini_string_to_array(iniConfig.get<std::string>("DEC.CityLocations", ""));
			CityChanceMultiplier = iniConfig.get<double>("DEC.CityChanceMultiplier", 1.0);

			WildernessLocations = ini_string_to_array(iniConfig.get<std::string>("DEC.WildernessLocations", ""));
			WildernessChanceMultiplier = iniConfig.get<double>("DEC.WildernessChanceMultiplier", 1.0);

			DungeonLocations = ini_string_to_array(iniConfig.get<std::string>("DEC.DungeonLocations", ""));
			DungeonChanceMultiplier = iniConfig.get<double>("DEC.DungeonChanceMultiplier", 1.0);


			useItemChanceMultiplier = iniConfig.get<bool>("DEC.useItemChanceMultiplier", false);
			useCountOfItemsChanceMultiplier = iniConfig.get<bool>("DEC.useCountOfItemsChanceMultiplier", false);
            
            ScrollItemChanceMultiplier = iniConfig.get<double>("DEC.ScrollItemChanceMultiplier", 1.0);
			ArmorItemChanceMultiplier = iniConfig.get<double>("DEC.ArmorItemChanceMultiplier", 1.0);
			BookItemChanceMultiplier = iniConfig.get<double>("DEC.BookItemChanceMultiplier", 1.0);
			IngredientItemChanceMultiplier = iniConfig.get<double>("DEC.IngredientItemChanceMultiplier", 1.0);
			LightItemChanceMultiplier = iniConfig.get<double>("DEC.LightItemChanceMultiplier", 1.0);
			MiscItemChanceMultiplier = iniConfig.get<double>("DEC.MiscItemChanceMultiplier", 1.0);
			WeaponItemChanceMultiplier = iniConfig.get<double>("DEC.WeaponItemChanceMultiplier", 1.0);
			AmmoItemChanceMultiplier = iniConfig.get<double>("DEC.AmmoItemChanceMultiplier", 1.0);
			KeyMasterItemChanceMultiplier = iniConfig.get<double>("DEC.KeyMasterItemChanceMultiplier", 1.0);
			AlchemyItemChanceMultiplier = iniConfig.get<double>("DEC.AlchemyItemChanceMultiplier", 1.0);
			SoulGemItemChanceMultiplier = iniConfig.get<double>("DEC.SoulGemItemChanceMultiplier", 1.0);

            
			visualiseChance = iniConfig.get<bool>("DEC.visualiseChance", false);
			QuickLootLogger = iniConfig.get<bool>("DEC.QuickLootLogger", false);


            boost::property_tree::ptree items = iniConfig.get_child("ITEMS");
			std::string::size_type name_start_pos = 5;
            for (const auto item : items) {
				if (item.first.compare(0, name_start_pos, "Item_") == 0) {
					std::string::size_type name_end_pos = item.first.find("_", name_start_pos);
                    if (name_end_pos != std::string::npos) {
						std::string itemName = item.first.substr(name_start_pos, name_end_pos - name_start_pos);
						if (itemName.size() > 0) {
							std::string key = item.first.substr(name_end_pos);
							if (key == "_FormId") {
								rawItemDefinitions[itemName].formId = std::strtol(item.second.get_value<std::string>("").c_str(), NULL, 0);
							}
							if (key == "_PluginFile") {
								rawItemDefinitions[itemName].plugin = item.second.get_value<std::string>("");
							}
							if (key == "_ChanceMultiplier") {
								rawItemDefinitions[itemName].chance = item.second.get_value<double>(1.0);
							}
						}
                    }
                }
            }

		} catch (std::exception& ex) {
			WARN("ERROR LOADING ini FILE: {}", ex.what());
		}
	}
}
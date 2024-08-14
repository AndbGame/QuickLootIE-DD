#include "Config.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace QuickLootDD
{

	static std::vector<std::string> ini_string_to_array(std::string value)
	{
		std::vector<std::string> loc_res;
        if (value.size() == 0) {
			return loc_res;
        }
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
		std::string mainFile = "Data\\skse\\plugins\\QuickLootIEDD.ini";

		if (std::filesystem::exists(mainFile)) {
			readIniConfig(mainFile);
		}

		const std::filesystem::path configPath{ "Data\\skse\\plugins\\QuickLootIEDD" };

        std::vector<std::string> configs;

        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{ configPath, std::filesystem::directory_options::skip_permission_denied }) {
			if (dir_entry.is_regular_file() && dir_entry.exists()) {
				if (dir_entry.path().extension().string() == ".ini") {
					configs.push_back(dir_entry.path().string());
                }
            }
		}
		std::sort(configs.begin(), configs.end(), [](std::string a, std::string b) { return a < b; });
        for (auto const& filename : configs) {
			readIniConfig(filename);
        }

	}

#define LOAD_PROPERTY_TREE(NAME, TYPE, CONFIG)                      \
	{                                                               \
		auto NAME##Optional = iniConfig.get_optional<TYPE>(CONFIG); \
		if (NAME##Optional.has_value()) {                           \
			NAME = NAME##Optional.value();                          \
		}                                                           \
	}

#define LOAD_PROPERTY_TREE_STRING_ARRAY(NAME, CONFIG)                       \
	{                                                                       \
		auto NAME##Optional = iniConfig.get_optional<std::string>(CONFIG);  \
		if (NAME##Optional.has_value()) {                                   \
			NAME = ini_string_to_array(NAME##Optional.value());             \
		}                                                                   \
	}

	void Config::readIniConfig(std::string filename)
	{
		auto iniConfig = boost::property_tree::ptree();
		try {
			boost::property_tree::ini_parser::read_ini(filename, iniConfig);

			LOG("readIniConfig: {}", filename);
			try {
				/* MAIN */
				LOAD_PROPERTY_TREE(logLevel, std::string, "MAIN.logLevel");
				LOAD_PROPERTY_TREE(baseChanceMin, double, "MAIN.baseChanceMin");
				if (baseChanceMin < 0) {
					WARN("readIniConfig: incorrect MAIN.baseChanceMin, adjusted to `0.0` in {}", filename);
					baseChanceMin = 0.0;
				}
				if (baseChanceMin > 1) {
					WARN("readIniConfig: incorrect MAIN.baseChanceMin, adjusted to `1.0` in {}", filename);
					baseChanceMin = 1.0;
				}

				LOAD_PROPERTY_TREE(baseChanceMax, double, "MAIN.baseChanceMax");
				if (baseChanceMax < baseChanceMin) {
					WARN("readIniConfig: incorrect MAIN.baseChanceMax, adjusted to `MAIN.baseChanceMin` in {}", filename);
					baseChanceMax = baseChanceMin;
				}

                LOAD_PROPERTY_TREE(chanceLimit, double, "MAIN.chanceLimit");
				if (chanceLimit < 0) {
					WARN("readIniConfig: incorrect MAIN.chanceLimit, adjusted to `0.0` in {}", filename);
					chanceLimit = 0.0;
				}
				if (baseChanceMin > 1) {
					WARN("readIniConfig: incorrect MAIN.chanceLimit, adjusted to `1.0` in {}", filename);
					chanceLimit = 1.0;
				}

                LOAD_PROPERTY_TREE(globalTriggerCooldown, int, "MAIN.globalTriggerCooldown");
				LOAD_PROPERTY_TREE(containerTriggerCooldown, int, "MAIN.containerTriggerCooldown");

                
                LOAD_PROPERTY_TREE(increaseChanceMultiplierMin, double, "MAIN.increaseChanceMultiplierMin");
				if (increaseChanceMultiplierMin < 1) {
					WARN("readIniConfig: incorrect MAIN.increaseChanceMultiplierMin, adjusted to `1.0` in {}", filename);
					increaseChanceMultiplierMin = 1.0;
				}

                LOAD_PROPERTY_TREE(increaseChanceMultiplierMax, double, "MAIN.increaseChanceMultiplierMax");
				if (increaseChanceMultiplierMax < increaseChanceMultiplierMin) {
					WARN("readIniConfig: incorrect MAIN.increaseChanceMultiplierMax, adjusted to `MAIN.increaseChanceMultiplierMin` in {}", filename);
					increaseChanceMultiplierMax = increaseChanceMultiplierMin;
				}

                LOAD_PROPERTY_TREE(containerChanceCooldown, int, "MAIN.containerChanceCooldown");
				LOAD_PROPERTY_TREE(resetChanceOnLastItem, bool, "MAIN.resetChanceOnLastItem");

				LOAD_PROPERTY_TREE(containerLimit, int, "MAIN.containerLimit");
				LOAD_PROPERTY_TREE(containerMaxLimitForInvalidate, int, "MAIN.containerMaxLimitForInvalidate");
				if (containerMaxLimitForInvalidate < containerLimit) {
					if (containerMaxLimitForInvalidate != 0) {
						WARN("readIniConfig: incorrect MAIN.containerMaxLimitForInvalidate, adjusted to {} in {}", containerLimit * 2, filename);
                    }
					containerMaxLimitForInvalidate = containerLimit * 2;
				}

                LOAD_PROPERTY_TREE_STRING_ARRAY(SafeLocations, "MAIN.SafeLocations");

				LOAD_PROPERTY_TREE_STRING_ARRAY(CityLocations, "MAIN.CityLocations");
				LOAD_PROPERTY_TREE(CityChanceMultiplier, double, "MAIN.CityChanceMultiplier");

				LOAD_PROPERTY_TREE_STRING_ARRAY(WildernessLocations, "MAIN.WildernessLocations");
				LOAD_PROPERTY_TREE(WildernessChanceMultiplier, double, "MAIN.WildernessChanceMultiplier");

				LOAD_PROPERTY_TREE_STRING_ARRAY(DungeonLocations, "MAIN.DungeonLocations");
				LOAD_PROPERTY_TREE(DungeonChanceMultiplier, double, "MAIN.DungeonChanceMultiplier");

                
				LOAD_PROPERTY_TREE(allowDeadBody, bool, "MAIN.allowDeadBody");
				LOAD_PROPERTY_TREE(allowActorTypeNPC, bool, "MAIN.allowActorTypeNPC");
				LOAD_PROPERTY_TREE(allowActorTypeCreature, bool, "MAIN.allowActorTypeCreature");
				LOAD_PROPERTY_TREE(allowActorTypeAnimal, bool, "MAIN.allowActorTypeAnimal");
				LOAD_PROPERTY_TREE(allowActorTypeDragon, bool, "MAIN.allowActorTypeDragon");
				LOAD_PROPERTY_TREE(allowOther, bool, "MAIN.allowOther");

				LOAD_PROPERTY_TREE(useItemChanceMultiplier, bool, "MAIN.useItemChanceMultiplier");
				LOAD_PROPERTY_TREE(useCountOfItemsChanceMultiplier, bool, "MAIN.useCountOfItemsChanceMultiplier");

				LOAD_PROPERTY_TREE(ScrollItemChanceMultiplier, double, "MAIN.ScrollItemChanceMultiplier");
				LOAD_PROPERTY_TREE(ArmorItemChanceMultiplier, double, "MAIN.ArmorItemChanceMultiplier");
				LOAD_PROPERTY_TREE(BookItemChanceMultiplier, double, "MAIN.BookItemChanceMultiplier");
				LOAD_PROPERTY_TREE(IngredientItemChanceMultiplier, double, "MAIN.IngredientItemChanceMultiplier");
				LOAD_PROPERTY_TREE(LightItemChanceMultiplier, double, "MAIN.LightItemChanceMultiplier");
				LOAD_PROPERTY_TREE(MiscItemChanceMultiplier, double, "MAIN.MiscItemChanceMultiplier");
				LOAD_PROPERTY_TREE(WeaponItemChanceMultiplier, double, "MAIN.WeaponItemChanceMultiplier");
				LOAD_PROPERTY_TREE(AmmoItemChanceMultiplier, double, "MAIN.AmmoItemChanceMultiplier");
				LOAD_PROPERTY_TREE(KeyMasterItemChanceMultiplier, double, "MAIN.KeyMasterItemChanceMultiplier");
				LOAD_PROPERTY_TREE(AlchemyItemChanceMultiplier, double, "MAIN.AlchemyItemChanceMultiplier");
				LOAD_PROPERTY_TREE(SoulGemItemChanceMultiplier, double, "MAIN.SoulGemItemChanceMultiplier");

				LOAD_PROPERTY_TREE(QuickLootLogger, bool, "MAIN.QuickLootLogger");
				LOAD_PROPERTY_TREE(useCoSave, bool, "MAIN.useCoSave");
				LOAD_PROPERTY_TREE(reloadConfigOnLoadSave, bool, "MAIN.reloadConfigOnLoadSave");

				LOAD_PROPERTY_TREE(visualiseChance, bool, "MAIN.visualiseChance");
				LOAD_PROPERTY_TREE(visualiseOverlayFile, std::string, "MAIN.visualiseOverlayFile");
				LOAD_PROPERTY_TREE(visualiseChanceVerbose, bool, "MAIN.visualiseChanceVerbose");
				LOAD_PROPERTY_TREE(visualiseMinIntensity, float, "MAIN.visualiseMinIntensity");
				if (visualiseMinIntensity < 0) {
					WARN("readIniConfig: incorrect MAIN.visualiseMinIntensity, adjusted to `1` in {}", filename);
					visualiseMinIntensity = 1;
				}

				LOAD_PROPERTY_TREE(visualiseMaxIntensity, float, "MAIN.visualiseMaxIntensity");
				if (visualiseMaxIntensity < visualiseMinIntensity) {
					visualiseMaxIntensity = visualiseMinIntensity;
					WARN("readIniConfig: incorrect MAIN.visualiseMaxIntensity, adjusted to {} in {}", visualiseMinIntensity, filename);
				}

				LOAD_PROPERTY_TREE(visualiseColorR, float, "MAIN.visualiseColorR");
				if (visualiseColorR < 0 || visualiseColorR > 255) {
					visualiseColorR = 255;
					WARN("readIniConfig: incorrect MAIN.visualiseColorR, adjusted to {} in {}", visualiseColorR, filename);
				}
				LOAD_PROPERTY_TREE(visualiseColorG, float, "MAIN.visualiseColorG");
				if (visualiseColorG < 0 || visualiseColorG > 255) {
					visualiseColorG = 255;
					WARN("readIniConfig: incorrect MAIN.visualiseColorR, adjusted to {} in {}", visualiseColorG, filename);
				}
				LOAD_PROPERTY_TREE(visualiseColorB, float, "MAIN.visualiseColorB");
				if (visualiseColorB < 0 || visualiseColorB > 255) {
					visualiseColorB = 255;
					WARN("readIniConfig: incorrect MAIN.visualiseColorR, adjusted to {} in {}", visualiseColorB, filename);
				}
				LOAD_PROPERTY_TREE(visualiseColorAmin, float, "MAIN.visualiseColorAmin");
				if (visualiseColorAmin < 0 || visualiseColorAmin > 1.0) {
					visualiseColorAmin = 0.0f;
					WARN("readIniConfig: incorrect MAIN.visualiseColorA, adjusted to {} in {}", visualiseColorAmin, filename);
				}
				LOAD_PROPERTY_TREE(visualiseColorAmax, float, "MAIN.visualiseColorAmax");
				if (visualiseColorAmax < visualiseColorAmin || visualiseColorAmax > 1.0) {
					visualiseColorAmax = 1.0f;
					WARN("readIniConfig: incorrect MAIN.visualiseColorA, adjusted to {} in {}", visualiseColorAmax, filename);
				}
			} catch (std::exception& ex) {
				ERROR("ERROR in [MAIN] section in ini file: {} - {}", filename, ex.what());
			}

			try {
				/* DD */
				LOAD_PROPERTY_TREE(RestrictLootMenu, bool, "DD.RestrictLootMenu");
			} catch (std::exception& ex) {
				ERROR("ERROR in [DD] section in ini file: {} - {}", filename, ex.what());
			}

			try {
				/* DEC */
				LOAD_PROPERTY_TREE(useDEC, bool, "DEC.useDEC");
				LOAD_PROPERTY_TREE(useDECContainerList, bool, "DEC.useDECContainerList");
			} catch (std::exception& ex) {
				ERROR("ERROR in [DEC] section in ini file: {} - {}", filename, ex.what());
			}
            
			for (const auto& section : iniConfig) {
				if (section.second.empty()) {
					continue;
				}
				try {
					std::string::size_type name_start_pos = 11;
					if (section.first.compare(0, name_start_pos, "BONUS_LOOT_") == 0) {
						std::string itemName = section.first.substr(name_start_pos);
						if (itemName.size() == 0) {
							continue;
						}
						auto FormIdOptional = section.second.get_optional<std::string>("FormId");
						if (FormIdOptional.has_value()) {
							bonusItemDefinition[itemName].formId = std::strtol(FormIdOptional.value().c_str(), NULL, 0);
						}
						auto PluginFileOptional = section.second.get_optional<std::string>("PluginFile");
						if (PluginFileOptional.has_value()) {
							bonusItemDefinition[itemName].plugin = PluginFileOptional.value();
						}
						auto MinCountOptional = section.second.get_optional<std::int32_t>("MinCount");
						if (MinCountOptional.has_value()) {
							bonusItemDefinition[itemName].minCount = MinCountOptional.value();
						}
						auto MaxCountOptional = section.second.get_optional<std::int32_t>("MaxCount");
						if (MaxCountOptional.has_value()) {
							bonusItemDefinition[itemName].maxCount = MaxCountOptional.value();
						}
						if (bonusItemDefinition[itemName].maxCount < bonusItemDefinition[itemName].minCount) {
							bonusItemDefinition[itemName].maxCount = bonusItemDefinition[itemName].minCount;
						}
						auto ChanceOptional = section.second.get_optional<double>("Chance");
						if (ChanceOptional.has_value()) {
							bonusItemDefinition[itemName].chance = ChanceOptional.value();
						}
						if (bonusItemDefinition[itemName].chance < 0) {
							bonusItemDefinition[itemName].chance = 0.0;
						}
						auto RequirementOptional = section.second.get_optional<std::string>("Requirement");
						if (RequirementOptional.has_value()) {
							auto req = ini_string_to_array(RequirementOptional.value());
							for (const std::string _r : req) {
								if (_r == "Lockable") {
									bonusItemDefinition[itemName].requirement.set(BonusItemDefinition::RequirementFlags::Lockable);
								}
								if (_r == "Belt") {
									bonusItemDefinition[itemName].requirement.set(BonusItemDefinition::RequirementFlags::Belt);
								}
								if (_r == "Bra") {
									bonusItemDefinition[itemName].requirement.set(BonusItemDefinition::RequirementFlags::Bra);
								}
								if (_r == "Plug") {
									bonusItemDefinition[itemName].requirement.set(BonusItemDefinition::RequirementFlags::Plug);
								}
								if (_r == "Piercing") {
									bonusItemDefinition[itemName].requirement.set(BonusItemDefinition::RequirementFlags::Piercing);
								}
								if (_r == "HeavyBondage") {
									bonusItemDefinition[itemName].requirement.set(BonusItemDefinition::RequirementFlags::HeavyBondage);
								}
								if (_r == "BondageMittens") {
									bonusItemDefinition[itemName].requirement.set(BonusItemDefinition::RequirementFlags::BondageMittens);
								}
							}
						}
					}

					name_start_pos = 13;
					if (section.first.compare(0, name_start_pos, "ITEMS_CHANCE_") == 0) {
						std::string itemName = section.first.substr(name_start_pos);
						if (itemName.size() == 0) {
							continue;
						}
						auto FormIdOptional = section.second.get_optional<std::string>("FormId");
						if (FormIdOptional.has_value()) {
							rawItemDefinitions[itemName].formId = std::strtol(FormIdOptional.value().c_str(), NULL, 0);
						}
						auto PluginFileOptional = section.second.get_optional<std::string>("PluginFile");
						if (PluginFileOptional.has_value()) {
							rawItemDefinitions[itemName].plugin = PluginFileOptional.value();
						}
						auto ChanceMultiplierOptional = section.second.get_optional<double>("ChanceMultiplier");
						if (ChanceMultiplierOptional.has_value()) {
							rawItemDefinitions[itemName].chance = ChanceMultiplierOptional.value();
						}
						if (rawItemDefinitions[itemName].chance < 0) {
							rawItemDefinitions[itemName].chance = 0.0;
						}
					}
				} catch (std::exception& ex) {
					ERROR("ERROR in [{}] section in ini file: {} - {}", section.first, filename, ex.what());
				}
			}

		} catch (std::exception& ex) {
			WARN("ERROR LOADING ini FILE: {} - {}", filename, ex.what());
		}
	}
}
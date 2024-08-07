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
		auto iniConfig = boost::property_tree::ptree();
		try {
			boost::property_tree::ini_parser::read_ini("Data\\skse\\plugins\\QuickLootIEDD.ini", iniConfig);

            try {
			    /* MAIN */
			    baseChanceMin = iniConfig.get<double>("MAIN.baseChanceMin", 0.01);
                if (baseChanceMin < 0) {
				    WARN("readIniConfig: incorrect MAIN.baseChanceMin, adjusted to `0.0`");
				    baseChanceMin = 0.0;
			    }
			    if (baseChanceMin > 1) {
				    WARN("readIniConfig: incorrect MAIN.baseChanceMin, adjusted to `1.0`");
				    baseChanceMin = 1.0;
			    }

			    baseChanceMax = iniConfig.get<double>("MAIN.baseChanceMax", 0.1);
			    if (baseChanceMax < baseChanceMin) {
				    WARN("readIniConfig: incorrect MAIN.baseChanceMax, adjusted to `MAIN.baseChanceMin`");
				    baseChanceMax = baseChanceMin;
			    }

			    chanceLimit = iniConfig.get<double>("MAIN.chanceLimit", 0.5);
			    if (chanceLimit < 0) {
				    WARN("readIniConfig: incorrect MAIN.chanceLimit, adjusted to `0.0`");
				    chanceLimit = 0.0;
			    }
			    if (baseChanceMin > 1) {
				    WARN("readIniConfig: incorrect MAIN.chanceLimit, adjusted to `1.0`");
				    chanceLimit = 1.0;
			    }

			    globalTriggerCooldown = iniConfig.get<int>("MAIN.containerTriggerCooldown", 60);
			    containerTriggerCooldown = iniConfig.get<int>("MAIN.containerTriggerCooldown", 360);

			    increaseChanceMultiplierMin = iniConfig.get<double>("MAIN.increaseChanceMultiplierMin", 1.01);
			    if (increaseChanceMultiplierMin < 1) {
				    WARN("readIniConfig: incorrect MAIN.increaseChanceMultiplierMin, adjusted to `1.0`");
				    increaseChanceMultiplierMin = 1.0;
			    }

			    increaseChanceMultiplierMax = iniConfig.get<double>("MAIN.increaseChanceMultiplierMax", 1.1);
			    if (increaseChanceMultiplierMax < increaseChanceMultiplierMin) {
				    WARN("readIniConfig: incorrect MAIN.increaseChanceMultiplierMax, adjusted to `MAIN.increaseChanceMultiplierMin`");
				    increaseChanceMultiplierMax = increaseChanceMultiplierMin;
			    }

			    containerChanceCooldown = iniConfig.get<int>("MAIN.containerChanceCooldown", 3600);
			    resetChanceOnLastItem = iniConfig.get<bool>("MAIN.resetChanceOnLastItem", true);

			    containerLimit = iniConfig.get<int>("MAIN.containerLimit", 100);
			    containerMaxLimitForInvalidate = iniConfig.get<int>("MAIN.containerMaxLimitForInvalidate", 0);
			    if (containerMaxLimitForInvalidate < containerLimit) {
				    containerMaxLimitForInvalidate = containerLimit * 2;
				    WARN("readIniConfig: incorrect MAIN.containerMaxLimitForInvalidate, adjusted to {}", containerMaxLimitForInvalidate);
                }

                SafeLocations = ini_string_to_array(iniConfig.get<std::string>("MAIN.SafeLocations", ""));

			    CityLocations = ini_string_to_array(iniConfig.get<std::string>("MAIN.CityLocations", ""));
			    CityChanceMultiplier = iniConfig.get<double>("MAIN.CityChanceMultiplier", 1.0);

			    WildernessLocations = ini_string_to_array(iniConfig.get<std::string>("MAIN.WildernessLocations", ""));
			    WildernessChanceMultiplier = iniConfig.get<double>("MAIN.WildernessChanceMultiplier", 1.0);

			    DungeonLocations = ini_string_to_array(iniConfig.get<std::string>("MAIN.DungeonLocations", ""));
			    DungeonChanceMultiplier = iniConfig.get<double>("MAIN.DungeonChanceMultiplier", 1.0);

			    allowDeadBody = iniConfig.get<bool>("MAIN.allowDeadBody", false);
			    allowActorTypeNPC = iniConfig.get<bool>("MAIN.allowActorTypeNPC", false);
			    allowActorTypeCreature = iniConfig.get<bool>("MAIN.allowActorTypeCreature", false);
			    allowActorTypeAnimal = iniConfig.get<bool>("MAIN.allowActorTypeAnimal", false);
			    allowActorTypeDragon = iniConfig.get<bool>("MAIN.allowActorTypeDragon", false);
			    allowOther = iniConfig.get<bool>("MAIN.allowOther", false);

			    useItemChanceMultiplier = iniConfig.get<bool>("MAIN.useItemChanceMultiplier", false);
			    useCountOfItemsChanceMultiplier = iniConfig.get<bool>("MAIN.useCountOfItemsChanceMultiplier", false);
            
                ScrollItemChanceMultiplier = iniConfig.get<double>("MAIN.ScrollItemChanceMultiplier", 1.0);
			    ArmorItemChanceMultiplier = iniConfig.get<double>("MAIN.ArmorItemChanceMultiplier", 1.0);
			    BookItemChanceMultiplier = iniConfig.get<double>("MAIN.BookItemChanceMultiplier", 1.0);
			    IngredientItemChanceMultiplier = iniConfig.get<double>("MAIN.IngredientItemChanceMultiplier", 1.0);
			    LightItemChanceMultiplier = iniConfig.get<double>("MAIN.LightItemChanceMultiplier", 1.0);
			    MiscItemChanceMultiplier = iniConfig.get<double>("MAIN.MiscItemChanceMultiplier", 1.0);
			    WeaponItemChanceMultiplier = iniConfig.get<double>("MAIN.WeaponItemChanceMultiplier", 1.0);
			    AmmoItemChanceMultiplier = iniConfig.get<double>("MAIN.AmmoItemChanceMultiplier", 1.0);
			    KeyMasterItemChanceMultiplier = iniConfig.get<double>("MAIN.KeyMasterItemChanceMultiplier", 1.0);
			    AlchemyItemChanceMultiplier = iniConfig.get<double>("MAIN.AlchemyItemChanceMultiplier", 1.0);
			    SoulGemItemChanceMultiplier = iniConfig.get<double>("MAIN.SoulGemItemChanceMultiplier", 1.0);

			    visualiseChance = iniConfig.get<bool>("MAIN.visualiseChance", false);
			    QuickLootLogger = iniConfig.get<bool>("MAIN.QuickLootLogger", false);
			    useCoSave = iniConfig.get<bool>("MAIN.useCoSave", true);
			    reloadConfigOnLoadSave = iniConfig.get<bool>("MAIN.reloadConfigOnLoadSave", false);
			} catch (std::exception& ex) {
				ERROR("ERROR in [MAIN] section in ini file: {}", ex.what());
			}
			
            try {
			    /* DD */
			    RestrictLootMenu = iniConfig.get<bool>("DD.RestrictLootMenu", true);
			} catch (std::exception& ex) {
				ERROR("ERROR in [DD] section in ini file: {}", ex.what());
			}

            try {
				/* DEC */
				useDEC = iniConfig.get<bool>("DEC.useDEC", true);
				useDECContainerList = iniConfig.get<bool>("DEC.useDECContainerList", true);
			} catch (std::exception& ex) {
				ERROR("ERROR in [DEC] section in ini file: {}", ex.what());
			}

			try { /* ITEMS_CHANCE_MULTIPLIER */
				boost::property_tree::ptree items = iniConfig.get_child("ITEMS_CHANCE_MULTIPLIER");
				std::string::size_type name_start_pos = 5;
				for (const auto& item : items) {
					try {
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
					} catch (std::exception& ex) {
						ERROR("ERROR in [ITEMS_CHANCE_MULTIPLIER].{} section in ini file: {}", item.first, ex.what());
					}
				}
			} catch (std::exception& ex) {
				ERROR("ERROR in [ITEMS_CHANCE_MULTIPLIER] section in ini file: {}", ex.what());
			}

			try { /* BONUS_LOOT */
				boost::property_tree::ptree items = iniConfig.get_child("BONUS_LOOT");
				std::string::size_type name_start_pos = 5;
				for (const auto& item : items) {
					try {
						if (item.first.compare(0, name_start_pos, "Item_") == 0) {
							std::string::size_type name_end_pos = item.first.find("_", name_start_pos);
							if (name_end_pos != std::string::npos) {
								std::string itemName = item.first.substr(name_start_pos, name_end_pos - name_start_pos);
								if (itemName.size() > 0) {
									std::string key = item.first.substr(name_end_pos);
									if (key == "_FormId") {
										bonusItemDefinition[itemName].formId = std::strtol(item.second.get_value<std::string>("").c_str(), NULL, 0);
									}
									if (key == "_PluginFile") {
										bonusItemDefinition[itemName].plugin = item.second.get_value<std::string>("");
									}
									if (key == "_MinCount") {
										bonusItemDefinition[itemName].minCount = item.second.get_value<std::int32_t>(1);
									}
									if (key == "_MaxCount") {
										bonusItemDefinition[itemName].maxCount = item.second.get_value<std::int32_t>(1);
									}
									if (bonusItemDefinition[itemName].maxCount < bonusItemDefinition[itemName].minCount) {
										bonusItemDefinition[itemName].maxCount = bonusItemDefinition[itemName].minCount;
									}
									if (key == "_Chance") {
										bonusItemDefinition[itemName].chance = item.second.get_value<double>(0.0);
									}
									if (key == "_Requirement") {
										auto req = ini_string_to_array(item.second.get_value<std::string>("ANY"));
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
							}
						}
					} catch (std::exception& ex) {
						ERROR("ERROR in [BONUS_LOOT].{} section in ini file: {}", item.first, ex.what());
					}
				}
			} catch (std::exception& ex) {
				ERROR("ERROR in [BONUS_LOOT] section in ini file: {}", ex.what());
			}

		} catch (std::exception& ex) {
			WARN("ERROR LOADING ini FILE: {}", ex.what());
		}
	}
}
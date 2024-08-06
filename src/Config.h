#pragma once

namespace QuickLootDD
{
	class Config
	{
	public:
		struct RawItemDefinition
		{
			RE::FormID formId = 0;
			std::string plugin = "";
			double chance = 1.0;
		};
		static inline std::map<std::string, RawItemDefinition> rawItemDefinitions;

		struct BonusItemDefinition
		{
			enum RequirementFlags
			{
				ANY = 1 << 0,
				HAS_RESTRAINT = 1 << 1,
				HAS_CHASTITY = 1 << 2,
				HAS_PIERCING = 1 << 3,
			};
			RE::FormID formId = 0;
			std::string plugin = "";
			size_t count = 1;
			double simpleChance = 0;
			double chanceToReplaceRestraint = 0;
			RequirementFlags requirement = RequirementFlags::ANY;
		};
		static inline std::map<std::string, BonusItemDefinition> bonusItemDefinition;

		static inline bool allowDeadBody = false;
		static inline bool allowActorTypeNPC = false;
		static inline bool allowActorTypeCreature = false;
		static inline bool allowActorTypeAnimal = false;
		static inline bool allowActorTypeDragon = false;
		static inline bool allowOther = false;

		static inline double baseChanceMin = 0.1;
		static inline double baseChanceMax = 0.3;
		static inline double chanceLimit = 0.9;
		static inline int globalTriggerCooldown = 60;
		static inline int containerTriggerCooldown = 360;
		static inline double increaseChanceMultiplierMin = 1.01;
		static inline double increaseChanceMultiplierMax = 1.3;
		static inline int containerChanceCooldown = 60 * 30;
		static inline bool resetChanceOnLastItem = false;

		static inline int containerLimit = 100;
		static inline int containerMaxLimitForInvalidate = 100;
        
		static inline std::vector<std::string> SafeLocations = {};

		static inline std::vector<std::string> CityLocations = {};
		static inline double CityChanceMultiplier = 1.0;

		static inline std::vector<std::string> WildernessLocations = {};
		static inline double WildernessChanceMultiplier = 1.0;

		static inline std::vector<std::string> DungeonLocations = {};
		static inline double DungeonChanceMultiplier = 1.0;

		static inline bool useItemChanceMultiplier = true;
		static inline bool useCountOfItemsChanceMultiplier = true;

        static inline double ScrollItemChanceMultiplier = 1.0;
		static inline double ArmorItemChanceMultiplier = 1.0;
		static inline double BookItemChanceMultiplier = 1.0;
		static inline double IngredientItemChanceMultiplier = 1.0;
		static inline double LightItemChanceMultiplier = 1.0;
		static inline double MiscItemChanceMultiplier = 1.0;
		static inline double WeaponItemChanceMultiplier = 1.0;
		static inline double AmmoItemChanceMultiplier = 1.0;
		static inline double KeyMasterItemChanceMultiplier = 1.0;
		static inline double AlchemyItemChanceMultiplier = 1.0;
		static inline double SoulGemItemChanceMultiplier = 1.0;

		static inline bool visualiseChance = false;
		static inline bool QuickLootLogger = false;

		static inline bool useCoSave = true;
        
		static inline bool useDECContainerList = true;
		static inline bool useDEC = true;

		static void readIniConfig();

    protected:
	};
}
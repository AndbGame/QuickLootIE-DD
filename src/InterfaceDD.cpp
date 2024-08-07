#include "InterfaceDD.h"

namespace QuickLootDD
{
	bool InterfaceDeviousDevices::LoadForms()
	{
		zad_DeviousBelt = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("zad_DeviousBelt");
		zad_DeviousBra = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("zad_DeviousBra");

		zad_DeviousPlug = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("zad_DeviousPlug");
		zad_DeviousPlugVaginal = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("zad_DeviousPlugVaginal");
		zad_DeviousPlugAnal = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("zad_DeviousPlugAnal");
		zad_DeviousPiercingsNipple = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("zad_DeviousPiercingsNipple");
		zad_DeviousPiercingsVaginal = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("zad_DeviousPiercingsVaginal");

		zad_DeviousHeavyBondage = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("zad_DeviousHeavyBondage");
		zad_DeviousBondageMittens = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("zad_DeviousBondageMittens");

		zad_Lockable = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("zad_Lockable");

		return true;
	}
}
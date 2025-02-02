//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////
#include "otpch.h"

#include "combat.h"
#include "configmanager.h"
#include "game.h"
#include "creature.h"
#include "player.h"
#include "const.h"
#include "tools.h"
#include "weapons.h"
#include "party.h"
#include <sstream>

extern Game g_game;
extern ConfigManager g_config;
extern Weapons* g_weapons;

Combat::Combat()
{
	params.valueCallback = NULL;
	params.tileCallback = NULL;
	params.targetCallback = NULL;
	area = NULL;
	formulaType = FORMULA_UNDEFINED;
	mina = 0.0;
	minb = 0.0;
	maxa = 0.0;
	maxb = 0.0;
}

Combat::~Combat()
{
	for (std::list<const Condition*>::iterator it = params.conditionList.begin(); it != params.conditionList.end(); ++it)
	{
		delete(*it);
	}

	params.conditionList.clear();
	delete params.valueCallback;
	delete params.tileCallback;
	delete params.targetCallback;
	delete area;
}

bool Combat::getMinMaxValues(Creature* creature, Creature* target, int32_t& min, int32_t& max) const
{
	if (creature)
	{
		if (creature->getCombatValues(min, max))
		{
			return true;
		}
		else if (Player* player = creature->getPlayer())
		{
			if (params.valueCallback)
			{
				params.valueCallback->getMinMaxValues(player, min, max, params.useCharges);
				return true;
			}
			else
			{
				switch (formulaType)
				{
					case FORMULA_LEVELMAGIC:
					{
						max = (int32_t)((player->getLevel() + player->getMagicLevel() * 4) * 1. * mina + minb);
						min = (int32_t)((player->getLevel() + player->getMagicLevel() * 4) * 1. * maxa + maxb);
						Vocation* vocation = player->getVocation();

						if (vocation)
						{
							if (max > 0 && min > 0 && vocation->getHealingBaseDamage() != 1.0)
							{
								min = int32_t(min * vocation->getHealingBaseDamage());
								max = int32_t(max * vocation->getHealingBaseDamage());
							}
							else if (max < 0 && min < 0 && vocation->getMagicBaseDamage() != 1.0)
							{
								min = int32_t(min * vocation->getMagicBaseDamage());
								max = int32_t(max * vocation->getMagicBaseDamage());
							}
						}

						return true;
						break;
					}
					case FORMULA_SKILL:
					{
						Item* tool = player->getWeapon();
						const Weapon* weapon = g_weapons->getWeapon(tool);
						min = (int32_t)minb;

						if (weapon)
						{
							max = (int32_t)(weapon->getWeaponDamage(player, target, tool, true) * maxa + maxb);

							if (params.useCharges && tool->hasCharges() && g_config.getNumber(ConfigManager::REMOVE_WEAPON_CHARGES))
							{
								int32_t newCharge = std::max((int32_t)0, ((int32_t)tool->getCharges()) - 1);
								g_game.transformItem(tool, tool->getID(), newCharge);
							}
						}
						else
						{
							max = (int32_t)maxb;
						}

						return true;
						break;
					}
					case FORMULA_VALUE:
					{
						min = (int32_t)mina;
						max = (int32_t)maxa;
						return true;
						break;
					}
					default:
						min = 0;
						max = 0;
						return false;
						break;
				}

				//std::cout << "No callback set for combat" << std::endl;
			}
		}
	}

	if (formulaType == FORMULA_VALUE)
	{
		min = (int32_t)mina;
		max = (int32_t)maxa;
		return true;
	}

	return false;
}

void Combat::getCombatArea(const Position& centerPos, const Position& targetPos, const AreaCombat* area,
                           std::list<Tile*>& list)
{
	if (area)
	{
		area->getList(centerPos, targetPos, list);
	}
	else if (targetPos.x >= 0 && targetPos.x < 0xFFFF &&
	         targetPos.y >= 0 && targetPos.y < 0xFFFF &&
	         targetPos.z >= 0 && targetPos.z < MAP_MAX_LAYERS)
	{
		Tile* tile = g_game.getTile(targetPos.x, targetPos.y, targetPos.z);

		if (!tile)
		{
			// These tiles will never have anything on them
			tile = new StaticTile(targetPos.x, targetPos.y, targetPos.z);
			g_game.setTile(tile);
		}

		list.push_back(tile);
	}
}

CombatType_t Combat::ConditionToDamageType(const ConditionType_t& type)
{
	switch (type)
	{
		case CONDITION_FIRE:
			return COMBAT_FIREDAMAGE;
			break;
		case CONDITION_ENERGY:
			return COMBAT_ENERGYDAMAGE;
			break;
		case CONDITION_DROWN:
			return COMBAT_DROWNDAMAGE;
			break;
		case CONDITION_POISON:
			return COMBAT_EARTHDAMAGE;
			break;
		case CONDITION_FREEZING:
			return COMBAT_ICEDAMAGE;
			break;
		case CONDITION_DAZZLED:
			return COMBAT_HOLYDAMAGE;
			break;
		case CONDITION_CURSED:
			return COMBAT_DEATHDAMAGE;
			break;
		default:
			break;
	}

	return COMBAT_NONE;
}

ConditionType_t Combat::DamageToConditionType(const CombatType_t& type)
{
	switch (type)
	{
		case COMBAT_FIREDAMAGE:
			return CONDITION_FIRE;
			break;
		case COMBAT_ENERGYDAMAGE:
			return CONDITION_ENERGY;
			break;
		case COMBAT_DROWNDAMAGE:
			return CONDITION_DROWN;
			break;
		case COMBAT_EARTHDAMAGE:
			return CONDITION_POISON;
			break;
		case COMBAT_ICEDAMAGE:
			return CONDITION_FREEZING;
			break;
		case COMBAT_HOLYDAMAGE:
			return CONDITION_DAZZLED;
			break;
		case COMBAT_DEATHDAMAGE:
			return CONDITION_CURSED;
			break;
		default:
			break;
	}

	return CONDITION_NONE;
}

bool Combat::isPlayerCombat(const Creature* target)
{
	return(target && target->getPlayerInCharge());
}

ReturnValue Combat::canTargetCreature(const Player* player, const Creature* target)
{
	if (player == target)
	{
		return RET_YOUMAYNOTATTACKTHISPERSON;
	}

	if (!player->hasFlag(PlayerFlag_IgnoreProtectionZone))
	{
		//pz-zone
		if (player->getZone() == ZONE_PROTECTION)
		{
			return RET_YOUMAYNOTATTACKAPERSONWHILEINPROTECTIONZONE;
		}

		if (target->getZone() == ZONE_PROTECTION)
		{
			return RET_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE;
		}

		//nopvp-zone
		if (isPlayerCombat(target))
		{
			if (player->getZone() == ZONE_NOPVP)
			{
				return RET_ACTIONNOTPERMITTEDINANONPVPZONE;
			}

			if (target->getZone() == ZONE_NOPVP)
			{
				return RET_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE;
			}
		}
	}

	if (player->hasFlag(PlayerFlag_CannotUseCombat) || !target->isAttackable())
	{
		if (target->getPlayer())
		{
			return RET_YOUMAYNOTATTACKTHISPERSON;
		}
		else
		{
			return RET_YOUMAYNOTATTACKTHISCREATURE;
		}
	}

	if (const Player* targetPlayer = target->getPlayer())
	{
		if (player->hasSafeMode())
		{
			if (player->isPartner(targetPlayer) || player->isWarPartner(targetPlayer) ||
			        player->isGuildEnemy(targetPlayer))
			{
				return Combat::canDoCombat(player, targetPlayer);
			}

			if (targetPlayer->getSkull() == SKULL_NONE)
			{
				if (!Combat::isInPvpZone(player, targetPlayer))
				{
					return RET_TURNSECUREMODETOATTACKUNMARKEDPLAYERS;
				}
			}
		}
		else if (player->getSkull() == SKULL_BLACK)
		{
			if (targetPlayer->getSkull() == SKULL_NONE && !targetPlayer->hasAttacked(player))
			{
				return RET_YOUMAYNOTATTACKTHISPERSON;
			}
		}
	}

	return Combat::canDoCombat(player, target);
}

ReturnValue Combat::canDoCombat(const Creature* caster, const Tile* tile, bool isAggressive)
{
	if (tile->hasProperty(BLOCKPROJECTILE))
	{
		return RET_NOTENOUGHROOM;
	}

	if (tile->floorChange())
	{
		return RET_NOTENOUGHROOM;
	}

	if (tile->getTeleportItem())
	{
		return RET_NOTENOUGHROOM;
	}

	if (caster)
	{
		if (caster->getPosition().z < tile->getPosition().z)
		{
			return RET_FIRSTGODOWNSTAIRS;
		}

		if (caster->getPosition().z > tile->getPosition().z)
		{
			return RET_FIRSTGOUPSTAIRS;
		}

		if (const Player* player = caster->getPlayer())
		{
			if (player->hasFlag(PlayerFlag_IgnoreProtectionZone))
			{
				return RET_NOERROR;
			}
		}
	}

	//pz-zone
	if (isAggressive && tile->hasFlag(TILESTATE_PROTECTIONZONE))
	{
		return RET_ACTIONNOTPERMITTEDINPROTECTIONZONE;
	}

	return RET_NOERROR;
}

bool Combat::isInPvpZone(const Creature* attacker, const Creature* target)
{
	if (attacker->getZone() != ZONE_PVP)
	{
		return false;
	}

	if (target->getZone() != ZONE_PVP)
	{
		return false;
	}

	return true;
}

bool Combat::isUnjustKill(const Creature* attacker, const Creature* target)
{
	const Player* attackerPlayer = attacker->getPlayer();
	const Player* targetPlayer = target->getPlayer();

	if (attacker->isPlayerSummon())
	{
		attackerPlayer = attacker->getPlayerMaster();
	}

	if (!attackerPlayer ||
	        !targetPlayer ||
	        targetPlayer == attackerPlayer ||
	        attackerPlayer->hasFlag(PlayerFlag_NotGainInFight) ||
	        attackerPlayer->isPartner(targetPlayer) ||
	        attackerPlayer->isWarPartner(targetPlayer) ||
	        attackerPlayer->isGuildEnemy(targetPlayer) ||
	        Combat::isInPvpZone(attackerPlayer, targetPlayer) ||
	        targetPlayer->hasAttacked(attackerPlayer) ||
	        targetPlayer->getSkull() != SKULL_NONE)
	{
		return false;
	}

	return true;
}

ReturnValue Combat::checkPVPExtraRestrictions(const Creature* attacker, const Creature* target, bool isWalkCheck)
{
#ifdef __MIN_PVP_LEVEL_APPLIES_TO_SUMMONS__
	const Player* targetPlayer;

	if (g_config.getNumber(ConfigManager::MIN_PVP_LEVEL_APPLIES_TO_SUMMONS))
	{
		targetPlayer = target->getPlayerInCharge();
	}
	else
	{
		targetPlayer = target->getPlayer();
	}

#else
	const Player* targetPlayer = target->getPlayer();
#endif
	const Player* attackerPlayer = attacker->getPlayerInCharge();

	if (targetPlayer && attackerPlayer)
	{
		bool stopAttack = false;
		bool canPassThrough = g_config.getBoolean(ConfigManager::CAN_PASS_THROUGH);

		if (g_game.getWorldType() == WORLD_TYPE_OPTIONAL_PVP)
		{
			if (!targetPlayer->isGuildEnemy(attackerPlayer) ||
			        (!isWalkCheck && !isInPvpZone(attacker, target)))
			{
				stopAttack = true;
			}

			if (isWalkCheck && !canPassThrough)
			{
				stopAttack = true;
			}
		}
		else
		{
			if (!isWalkCheck || canPassThrough)
			{
				uint32_t p_level = g_config.getNumber(ConfigManager::MIN_PVP_LEVEL);
				uint32_t attackerLevel = attackerPlayer->getLevel();
				uint32_t targetLevel = targetPlayer->getLevel();

				if ((attackerLevel >= p_level && targetLevel < p_level && isWalkCheck) ||
				        (!isWalkCheck && (attackerLevel < p_level || targetLevel < p_level)))
				{
					stopAttack = true;
				}
			}
		}

		if (stopAttack)
		{
			if (target->getPlayer())
			{
				return RET_YOUMAYNOTATTACKTHISPERSON;
			}
			else
			{
				return RET_YOUMAYNOTATTACKTHISCREATURE;
			}
		}
	}

	return RET_NOERROR;
}

ReturnValue Combat::canDoCombat(const Creature* attacker, const Creature* target)
{
	if (attacker)
	{
		if (const Player* targetPlayer = target->getPlayer())
		{
			if (targetPlayer->hasFlag(PlayerFlag_CannotBeAttacked))
			{
				return RET_YOUMAYNOTATTACKTHISPERSON;
			}

			if (const Player* attackerPlayer = attacker->getPlayer())
			{
				if (attackerPlayer->hasFlag(PlayerFlag_CannotAttackPlayer) ||
				        attackerPlayer->isLoginAttackLocked(targetPlayer->getID()))
				{
					return RET_YOUMAYNOTATTACKTHISPERSON;
				}

				if (attackerPlayer->getSkull() == SKULL_BLACK &&
				        !attackerPlayer->isGuildEnemy(targetPlayer) &&
				        !attackerPlayer->isPartner(targetPlayer) &&
				        !attackerPlayer->isWarPartner(targetPlayer))
				{
					if (targetPlayer->getSkull() == SKULL_NONE && !targetPlayer->hasAttacked(attackerPlayer))
					{
						return RET_YOUMAYNOTATTACKTHISPERSON;
					}
				}
			}

			if (const Player* masterAttackerPlayer = attacker->getPlayerMaster())
			{
				if (masterAttackerPlayer->hasFlag(PlayerFlag_CannotAttackPlayer))
				{
					return RET_YOUMAYNOTATTACKTHISPERSON;
				}
			}
		}
		else if (target->getMonster())
		{
			if (const Player* attackerPlayer = attacker->getPlayer())
			{
				if (attackerPlayer->hasFlag(PlayerFlag_CannotAttackMonster))
				{
					return RET_YOUMAYNOTATTACKTHISCREATURE;
				}
			}
		}

		if (attacker->getPlayer() || attacker->isPlayerSummon())
		{
			//nopvp-zone
			if (target->getPlayer() && target->getTile()->hasFlag(TILESTATE_NOPVPZONE))
			{
				return RET_ACTIONNOTPERMITTEDINANONPVPZONE;
			}

			return Combat::checkPVPExtraRestrictions(attacker, target, false);
		}
	}

	return RET_NOERROR;
}

void Combat::setPlayerCombatValues(const formulaType_t& _type, const double& _mina,
                                   const double& _minb, const double& _maxa, const double& _maxb)
{
	formulaType = _type;
	mina = _mina;
	minb = _minb;
	maxa = _maxa;
	maxb = _maxb;
}

void Combat::postCombatEffects(Creature* caster, const Position& pos) const
{
	postCombatEffects(caster, pos, params);
}

bool Combat::setParam(const CombatParam_t& param, const uint32_t& value)
{
	switch (param)
	{
		case COMBATPARAM_COMBATTYPE:
		{
			params.combatType = (CombatType_t)value;
			return true;
		}
		case COMBATPARAM_EFFECT:
		{
			params.impactEffect = (uint8_t)value;
			return true;
		}
		case COMBATPARAM_DISTANCEEFFECT:
		{
			params.distanceEffect = (uint8_t)value;
			return true;
		}
		case COMBATPARAM_BLOCKEDBYARMOR:
		{
			params.blockedByArmor = (value != 0);
			return true;
		}
		case COMBATPARAM_BLOCKEDBYSHIELD:
		{
			params.blockedByShield = (value != 0);
			return true;
		}
		case COMBATPARAM_TARGETCASTERORTOPMOST:
		{
			params.targetCasterOrTopMost = (value != 0);
			return true;
		}
		case COMBATPARAM_CREATEITEM:
		{
			params.itemId = value;
			return true;
		}
		case COMBATPARAM_AGGRESSIVE:
		{
			params.isAggressive = (value != 0);
			return true;
		}
		case COMBATPARAM_DISPEL:
		{
			params.dispelType = (ConditionType_t)value;
			return true;
		}
		case COMBATPARAM_USECHARGES:
		{
			params.useCharges = (value != 0);
			return true;
		}
		case COMBATPARAM_HITEFFECT:
		{
			params.hitEffect = (MagicEffectClasses)value;
			return true;
		}
		case COMBATPARAM_HITTEXTCOLOR:
		{
			params.hitTextColor = (TextColor_t)value;
			return true;
		}
		case COMBATPARAM_PZBLOCK:
		{
			params.pzBlock = (value != 0);
			return true;
		}
		default:
		{
			break;
		}
	}

	return false;
}

void Combat::setArea(AreaCombat* _area)
{
	if (area)
	{
		delete area;
	}

	area = _area;
}

bool Combat::hasArea() const
{
	return area;
}

void Combat::setCondition(const Condition* _condition)
{
	params.conditionList.push_back(_condition);
}

bool Combat::setCallback(const CallBackParam_t& key)
{
	switch (key)
	{
		case CALLBACKPARAM_LEVELMAGICVALUE:
		{
			delete params.valueCallback;
			params.valueCallback = new ValueCallback(FORMULA_LEVELMAGIC);
			return true;
		}
		case CALLBACKPARAM_SKILLVALUE:
		{
			delete params.valueCallback;
			params.valueCallback = new ValueCallback(FORMULA_SKILL);
			return true;
		}
		case CALLBACKPARAM_TARGETTILECALLBACK:
		{
			delete params.tileCallback;
			params.tileCallback = new TileCallback();
			return true;
		}
		case CALLBACKPARAM_TARGETCREATURECALLBACK:
		{
			delete params.targetCallback;
			params.targetCallback = new TargetCallback();
			return true;
		}
		default:
		{
			std::cout << "Combat::setCallback - Unknown callback type: " << (uint32_t)key << std::endl;
			return false;
		}
	}
}

CallBack* Combat::getCallback(const CallBackParam_t& key)
{
	switch (key)
	{
		case CALLBACKPARAM_LEVELMAGICVALUE:
		case CALLBACKPARAM_SKILLVALUE:
		{
			return params.valueCallback;
		}
		case CALLBACKPARAM_TARGETTILECALLBACK:
		{
			return params.tileCallback;
		}
		case CALLBACKPARAM_TARGETCREATURECALLBACK:
		{
			return params.targetCallback;
		}
	}

	return NULL;
}

void Combat::doPVPDamageReduction(int32_t& healthChange, const Player* target) //static
{
	if (healthChange < 0)
	{
		int64_t factor;

		if (target->getSkull() != SKULL_BLACK)
		{
			factor = std::max(g_config.getNumber(ConfigManager::PVP_DAMAGE), int64_t(0));
		}
		else
		{
			factor = std::max(g_config.getNumber(ConfigManager::PVP_DAMAGE_AT_BLACK_SKULLS), int64_t(0));
		}

		healthChange = (healthChange * factor) / 100;
	}
}

void Combat::checkPVPDamageReduction(const Creature* attacker, const Creature* target, int32_t& healthChange) //static
{
	if (attacker && attacker->getPlayer() && target->getPlayer() && (attacker != target))
	{
		Combat::doPVPDamageReduction(healthChange, target->getPlayer());
	}
}

bool Combat::CombatHealthFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	Combat2Var* var = (Combat2Var*)data;
	int32_t healthChange = random_range(var->minChange, var->maxChange, DISTRO_NORMAL);

	if (g_game.combatBlockHit(params.combatType, caster, target, healthChange, params.blockedByShield, params.blockedByArmor))
	{
		return false;
	}

	Combat::checkPVPDamageReduction(caster, target, healthChange);
	bool result = g_game.combatChangeHealth(params.combatType, params.hitEffect, params.hitTextColor, caster, target, healthChange);

	if (result)
	{
		CombatConditionFunc(caster, target, params, NULL);
		CombatDispelFunc(caster, target, params, NULL);
	}

	return result;
}

bool Combat::CombatManaFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	Combat2Var* var = (Combat2Var*)data;
	int32_t manaChange = random_range(var->minChange, var->maxChange, DISTRO_NORMAL);
	Combat::checkPVPDamageReduction(caster, target, manaChange);
	bool result = g_game.combatChangeMana(caster, target, manaChange);

	if (result)
	{
		CombatConditionFunc(caster, target, params, NULL);
		CombatDispelFunc(caster, target, params, NULL);
	}

	return result;
}

bool Combat::CombatConditionFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	bool result = false;

	if (!params.conditionList.empty())
	{
		for (std::list<const Condition*>::const_iterator it = params.conditionList.begin(); it != params.conditionList.end(); ++it)
		{
			const Condition* condition = *it;

			if (caster == target || !target->isImmune(condition->getType(), params.isAggressive))
			{
				Condition* conditionCopy = condition->clone();

				if (caster)
				{
					conditionCopy->setParam(CONDITIONPARAM_OWNER, caster->getID());
				}

				//TODO: infight condition until all aggressive conditions has ended
				result = target->addCombatCondition(conditionCopy);
			}
		}
	}

	return result;
}

bool Combat::CombatDispelFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	if (target->hasCondition(params.dispelType))
	{
		target->removeCondition(caster, params.dispelType);
		return true;
	}

	return false;
}

bool Combat::CombatNullFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	CombatConditionFunc(caster, target, params, NULL);
	CombatDispelFunc(caster, target, params, NULL);
	onCreatureDoCombat(caster, target, params.isAggressive);
	return true;
}

void Combat::combatTileEffects(const SpectatorVec& list, Creature* caster, Tile* tile, const CombatParams& params)
{
	if (params.itemId != 0)
	{
		uint32_t itemId = params.itemId;
		Player* p_caster = NULL;

		if (caster)
		{
			if (caster->getPlayer())
			{
				p_caster = caster->getPlayer();
			}
			else if (caster->isPlayerSummon())
			{
				p_caster = caster->getPlayerMaster();
			}
		}

		if (p_caster)
		{
			if (p_caster->getLevel() < g_config.getNumber(ConfigManager::MIN_PVP_LEVEL))
			{
				if (itemId == ITEM_WILDGROWTH)
				{
					if (g_config.getBoolean(ConfigManager::CAN_PASS_THROUGH) &&
					        g_config.getNumber(ConfigManager::MIN_PVP_LEVEL) > 0)
					{
						itemId = ITEM_WILDGROWTH_SAFE;
					}
				}

				if (itemId == ITEM_MAGICWALL)
				{
					if (g_config.getBoolean(ConfigManager::CAN_PASS_THROUGH) &&
					        g_config.getNumber(ConfigManager::MIN_PVP_LEVEL) > 0)
					{
						itemId = ITEM_MAGICWALL_SAFE;
					}
				}
			}

			if (g_game.getWorldType() == WORLD_TYPE_OPTIONAL_PVP || tile->hasFlag(TILESTATE_NOPVPZONE))
			{
				if (itemId == ITEM_FIREFIELD)
				{
					itemId = ITEM_FIREFIELD_SAFE;
				}
				else if (itemId == ITEM_POISONFIELD)
				{
					itemId = ITEM_POISONFIELD_SAFE;
				}
				else if (itemId == ITEM_ENERGYFIELD)
				{
					itemId = ITEM_ENERGYFIELD_SAFE;
				}
				else if (itemId == ITEM_MAGICWALL)
				{
					itemId = ITEM_MAGICWALL_SAFE;
				}
				else if (itemId == ITEM_WILDGROWTH)
				{
					itemId = ITEM_WILDGROWTH_SAFE;
				}
			}
		}

		Item* item = Item::CreateItem(itemId);

		if (caster)
		{
			item->setOwner(caster->getID());
		}

		ReturnValue ret = g_game.internalAddItem(tile, item);

		if (ret == RET_NOERROR)
		{
			g_game.startDecay(item);
		}
		else
		{
			delete item;
		}
	}

	if (params.tileCallback)
	{
		params.tileCallback->onTileCombat(caster, tile);
	}

	if (params.impactEffect != NM_ME_NONE)
	{
		g_game.addMagicEffect(list, tile->getPosition(), params.impactEffect);
	}
}

void Combat::postCombatEffects(Creature* caster, const Position& pos, const CombatParams& params)
{
	if (caster)
	{
		if (params.distanceEffect != NM_ME_NONE)
		{
			addDistanceEffect(caster, caster->getPosition(), pos, params.distanceEffect);
		}

		Player* p_caster = NULL;

		if (caster->getPlayer())
		{
			p_caster = caster->getPlayer();
		}
		else if (caster->isPlayerSummon())
		{
			p_caster = caster->getPlayerMaster();
		}

		if (p_caster && !p_caster->hasFlag(PlayerFlag_NotGainInFight) && params.isAggressive)
		{
			p_caster->addInFightTicks(g_config.getNumber(ConfigManager::IN_FIGHT_DURATION), params.pzBlock);
		}
	}
}

void Combat::addDistanceEffect(Creature* caster, const Position& fromPos,
                               const Position& toPos, const uint8_t& effect)
{
	uint8_t distanceEffect = effect;

	if (distanceEffect == NM_SHOOT_WEAPONTYPE)
	{
		switch (caster->getWeaponType())
		{
			case WEAPON_AXE:
				distanceEffect = NM_SHOOT_WHIRLWINDAXE;
				break;
			case WEAPON_SWORD:
				distanceEffect = NM_SHOOT_WHIRLWINDSWORD;
				break;
			case WEAPON_CLUB:
				distanceEffect = NM_SHOOT_WHIRLWINDCLUB;
				break;
			default:
				distanceEffect = NM_ME_NONE;
				break;
		}
	}

	if (caster && distanceEffect != NM_ME_NONE)
	{
		g_game.addDistanceEffect(fromPos, toPos, distanceEffect);
	}
}


void Combat::CombatFunc(Creature* caster, const Position& pos,
                        const AreaCombat* area, const CombatParams& params, COMBATFUNC func, void* data)
{
	std::list<Tile*> tileList;

	if (caster)
	{
		getCombatArea(caster->getPosition(), pos, area, tileList);
	}
	else
	{
		getCombatArea(pos, pos, area, tileList);
	}

	SpectatorVec list;
	uint32_t maxX = 0;
	uint32_t maxY = 0;
	uint32_t diff;

	//calculate the max viewable range
	for (std::list<Tile*>::iterator it = tileList.begin(); it != tileList.end(); ++it)
	{
		diff = std::abs((*it)->getPosition().x - pos.x);

		if (diff > maxX)
		{
			maxX = diff;
		}

		diff = std::abs((*it)->getPosition().y - pos.y);

		if (diff > maxY)
		{
			maxY = diff;
		}
	}

	g_game.getSpectators(list, pos, false, true, maxX + Map::maxViewportX, maxX + Map::maxViewportX,
	                     maxY + Map::maxViewportY, maxY + Map::maxViewportY);

	for (std::list<Tile*>::iterator it = tileList.begin(); it != tileList.end(); ++it)
	{
		Tile* iter_tile = *it;
		bool bContinue = true;

		if (canDoCombat(caster, iter_tile, params.isAggressive) == RET_NOERROR)
		{
			if (iter_tile->getCreatures())
			{
				for (CreatureVector::iterator cit = iter_tile->getCreatures()->begin(),
				        cend = iter_tile->getCreatures()->end();
				        bContinue && cit != cend; ++cit)
				{
					if (params.targetCasterOrTopMost)
					{
						if (caster && caster->getTile() == iter_tile)
						{
							if (*cit == caster)
							{
								bContinue = false;
							}
						}
						else if (*cit == iter_tile->getTopCreature())
						{
							bContinue = false;
						}

						if (bContinue)
						{
							continue;
						}
					}

					if (!params.isAggressive || (caster != *cit && Combat::canDoCombat(caster, *cit) == RET_NOERROR))
					{
						func(caster, *cit, params, data);

						if (params.targetCallback)
						{
							params.targetCallback->onTargetCombat(caster, *cit);
						}

						if (func == CombatDispelFunc || func == CombatConditionFunc)
						{
							onCreatureDoCombat(caster, *cit, params.isAggressive);
						}
					}
				}
			}

			combatTileEffects(list, caster, iter_tile, params);
		}
	}

	postCombatEffects(caster, pos, params);
}

void Combat::doCombat(Creature* caster, Creature* target) const
{
	//target combat callback function
	if (params.combatType != COMBAT_NONE)
	{
		int32_t minChange = 0;
		int32_t maxChange = 0;
		getMinMaxValues(caster, target, minChange, maxChange);

		if (params.combatType != COMBAT_MANADRAIN)
		{
			doCombatHealth(caster, target, minChange, maxChange, params);
		}
		else
		{
			doCombatMana(caster, target, minChange, maxChange, params);
		}
	}
	else
	{
		doCombatDefault(caster, target, params);
	}
}

void Combat::doCombat(Creature* caster, const Position& pos) const
{
	//area combat callback function
	if (params.combatType != COMBAT_NONE)
	{
		int32_t minChange = 0;
		int32_t maxChange = 0;
		getMinMaxValues(caster, NULL, minChange, maxChange);

		if (params.combatType != COMBAT_MANADRAIN)
		{
			doCombatHealth(caster, pos, area, minChange, maxChange, params);
		}
		else
		{
			doCombatMana(caster, pos, area, minChange, maxChange, params);
		}
	}
	else
	{
		CombatFunc(caster, pos, area, params, CombatNullFunc, NULL);
	}
}

void Combat::doCombatHealth(Creature* caster, Creature* target,
                            const int32_t& minChange, const int32_t& maxChange, const CombatParams& params)
{
	if (!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR))
	{
		Combat2Var var;
		var.minChange = minChange;
		var.maxChange = maxChange;
		CombatHealthFunc(caster, target, params, (void*)&var);

		if (params.impactEffect != NM_ME_NONE)
		{
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);
		}

		if (caster && params.distanceEffect != NM_ME_NONE)
		{
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
		}
	}
}

void Combat::doCombatHealth(Creature* caster, const Position& pos,
                            const AreaCombat* area, const int32_t& minChange, const int32_t& maxChange, const CombatParams& params)
{
	Combat2Var var;
	var.minChange = minChange;
	var.maxChange = maxChange;
	CombatFunc(caster, pos, area, params, CombatHealthFunc, (void*)&var);
}

void Combat::doCombatMana(Creature* caster, Creature* target,
                          const int32_t& minChange, const int32_t& maxChange, const CombatParams& params)
{
	if (!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR))
	{
		Combat2Var var;
		var.minChange = minChange;
		var.maxChange = maxChange;
		CombatManaFunc(caster, target, params, (void*)&var);

		if (params.targetCallback)
		{
			params.targetCallback->onTargetCombat(caster, target);
		}

		if (params.impactEffect != NM_ME_NONE)
		{
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);
		}

		if (caster && params.distanceEffect != NM_ME_NONE)
		{
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
		}
	}
}

void Combat::doCombatMana(Creature* caster, const Position& pos,
                          const AreaCombat* area, const int32_t& minChange, const int32_t& maxChange, const CombatParams& params)
{
	Combat2Var var;
	var.minChange = minChange;
	var.maxChange = maxChange;
	CombatFunc(caster, pos, area, params, CombatManaFunc, (void*)&var);
}

void Combat::doCombatCondition(Creature* caster, const Position& pos, const AreaCombat* area,
                               const CombatParams& params)
{
	CombatFunc(caster, pos, area, params, CombatConditionFunc, NULL);
}

void Combat::doCombatCondition(Creature* caster, Creature* target, const CombatParams& params)
{
	if (!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR))
	{
		CombatConditionFunc(caster, target, params, NULL);

		if (params.targetCallback)
		{
			params.targetCallback->onTargetCombat(caster, target);
		}

		if (params.impactEffect != NM_ME_NONE)
		{
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);
		}

		if (caster && params.distanceEffect != NM_ME_NONE)
		{
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
		}

		onCreatureDoCombat(caster, target, params.isAggressive);
	}
}

void Combat::doCombatDispel(Creature* caster, const Position& pos, const AreaCombat* area,
                            const CombatParams& params)
{
	CombatFunc(caster, pos, area, params, CombatDispelFunc, NULL);
}

void Combat::doCombatDispel(Creature* caster, Creature* target, const CombatParams& params)
{
	if (!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR))
	{
		CombatDispelFunc(caster, target, params, NULL);

		if (params.targetCallback)
		{
			params.targetCallback->onTargetCombat(caster, target);
		}

		if (params.impactEffect != NM_ME_NONE)
		{
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);
		}

		if (caster && params.distanceEffect != NM_ME_NONE)
		{
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
		}

		onCreatureDoCombat(caster, target, params.isAggressive);
	}
}

void Combat::doCombatDefault(Creature* caster, Creature* target, const CombatParams& params)
{
	if (!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR))
	{
		const SpectatorVec& list = g_game.getSpectators(target->getTile()->getPosition());
		CombatNullFunc(caster, target, params, NULL);
		combatTileEffects(list, caster, target->getTile(), params);

		if (params.targetCallback)
		{
			params.targetCallback->onTargetCombat(caster, target);
		}

		if (params.impactEffect != NM_ME_NONE)
		{
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);
		}

		if (caster && params.distanceEffect != NM_ME_NONE)
		{
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
		}
	}
}

void Combat::onCreatureDoCombat(Creature* caster, Creature* target, bool isAggressive)
{
	if (isAggressive && caster && target && caster != target)
	{
		caster->onAttackedCreature(target);
		target->onAttacked();
	}
}

//**********************************************************

ValueCallback::ValueCallback(const formulaType_t& _type)
	: type(_type)
{}

ValueCallback::~ValueCallback()
{
	// Virtual Destructor
}

void ValueCallback::getMinMaxValues(Player* player, int32_t& min, int32_t& max, bool useCharges) const
{
	//"onGetPlayerMinMaxValues"(...)
	if (m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
		lua_State* L = m_scriptInterface->getLuaState();

		if (!env->setCallbackId(m_scriptId, m_scriptInterface))
		{
			return;
		}

		uint32_t cid = env->addThing(player);
		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		int32_t parameters = 1;
		bool isMagicFormula = false;

		switch (type)
		{
			case FORMULA_LEVELMAGIC:
			{
				//"onGetPlayerMinMaxValues"(cid, level, maglevel)
				lua_pushnumber(L, player->getLevel());
				lua_pushnumber(L, player->getMagicLevel());
				parameters += 2;
				isMagicFormula = true;
				break;
			}
			case FORMULA_SKILL:
			{
				//"onGetPlayerMinMaxValues"(cid, attackSkill, attackValue, attackFactor)
				Item* tool = player->getWeapon();
				int32_t attackSkill = player->getWeaponSkill(tool);
				int32_t attackValue = 7;

				if (tool)
				{
					attackValue = tool->getAttack();

					if (useCharges && tool->hasCharges() && g_config.getNumber(ConfigManager::REMOVE_WEAPON_CHARGES))
					{
						int32_t newCharge = std::max(0, tool->getCharges() - 1);
						g_game.transformItem(tool, tool->getID(), newCharge);
					}
				}

				float attackFactor = player->getAttackFactor();
				lua_pushnumber(L, attackSkill);
				lua_pushnumber(L, attackValue);
				lua_pushnumber(L, attackFactor);
				parameters += 3;
				break;
			}
			default:
				std::cout << "ValueCallback::getMinMaxValues - unknown callback type" << std::endl;
				return;
				break;
		}

		int size0 = lua_gettop(L);

		if (lua_pcall(L, parameters, 2 /*nReturnValues*/, 0) != 0)
		{
			LuaScriptInterface::reportError(NULL, LuaScriptInterface::popString(L));
		}
		else
		{
			max = LuaScriptInterface::popNumber(L);
			min = LuaScriptInterface::popNumber(L);
			Vocation* vocation = player->getVocation();

			if (isMagicFormula && vocation)
			{
				if (max > 0 && min > 0 && vocation->getHealingBaseDamage() != 1.0)
				{
					min = int32_t(min * vocation->getHealingBaseDamage());
					max = int32_t(max * vocation->getHealingBaseDamage());
				}
				else if (max < 0 && min < 0 && vocation->getMagicBaseDamage() != 1.0)
				{
					min = int32_t(min * vocation->getMagicBaseDamage());
					max = int32_t(max * vocation->getMagicBaseDamage());
				}
			}
		}

		if ((lua_gettop(L) + parameters /*nParams*/  + 1) != size0)
		{
			LuaScriptInterface::reportError(NULL, "Stack size changed!");
		}

		env->resetCallback();
		m_scriptInterface->releaseScriptEnv();
	}
	else
	{
		std::cout << "[Error] Call stack overflow. ValueCallback::getMinMaxValues" << std::endl;
		return;
	}
}

//**********************************************************
TileCallback::~TileCallback()
{
	// Virtual Destructor
}

void TileCallback::onTileCombat(Creature* creature, Tile* tile) const
{
	//"onTileCombat"(cid, pos)
	if (m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
		lua_State* L = m_scriptInterface->getLuaState();

		if (!env->setCallbackId(m_scriptId, m_scriptInterface))
		{
			return;
		}

		uint32_t cid = 0;

		if (creature)
		{
			cid = env->addThing(creature);
		}

		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		m_scriptInterface->pushPosition(L, tile->getPosition(), 0);
		m_scriptInterface->callFunction(2, false);
		env->resetCallback();
		m_scriptInterface->releaseScriptEnv();
	}
	else
	{
		std::cout << "[Error] Call stack overflow. TileCallback::onTileCombat" << std::endl;
		return;
	}
}

//**********************************************************
TargetCallback::~TargetCallback()
{
	// Virtual Destructor
}

void TargetCallback::onTargetCombat(Creature* creature, Creature* target) const
{
	//"onTargetCombat"(cid, target)
	if (m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
		lua_State* L = m_scriptInterface->getLuaState();

		if (!env->setCallbackId(m_scriptId, m_scriptInterface))
		{
			return;
		}

		uint32_t cid = 0;

		if (creature)
		{
			cid = env->addThing(creature);
		}

		uint32_t targetCid = env->addThing(target);
		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		lua_pushnumber(L, targetCid);
		int size0 = lua_gettop(L);

		if (lua_pcall(L, 2, 0 /*nReturnValues*/, 0) != 0)
		{
			LuaScriptInterface::reportError(NULL, LuaScriptInterface::popString(L));
		}

		if ((lua_gettop(L) + 2 /*nParams*/  + 1) != size0)
		{
			LuaScriptInterface::reportError(NULL, "Stack size changed!");
		}

		env->resetCallback();
		m_scriptInterface->releaseScriptEnv();
	}
	else
	{
		std::cout << "[Error] Call stack overflow. TargetCallback::onTargetCombat" << std::endl;
		return;
	}
}

MatrixArea::MatrixArea(const uint32_t& _rows, const uint32_t& _cols)
{
	centerX = 0;
	centerY = 0;
	rows = _rows;
	cols = _cols;
	data_ = new bool*[rows];

	for (uint32_t row = 0; row < rows; ++row)
	{
		data_[row] = new bool[cols];

		for (uint32_t col = 0; col < cols; ++col)
		{
			data_[row][col] = 0;
		}
	}
}

MatrixArea::MatrixArea(const MatrixArea& rhs)
{
	centerX = rhs.centerX;
	centerY = rhs.centerY;
	rows = rhs.rows;
	cols = rhs.cols;
	data_ = new bool*[rows];

	for (uint32_t row = 0; row < rows; ++row)
	{
		data_[row] = new bool[cols];

		for (uint32_t col = 0; col < cols; ++col)
		{
			data_[row][col] = rhs.data_[row][col];
		}
	}
}

MatrixArea::~MatrixArea()
{
	for (uint32_t row = 0; row < rows; ++row)
	{
		delete[] data_[row];
	}

	delete[] data_;
}

void MatrixArea::setValue(const uint32_t& row, const uint32_t& col, bool value) const
{
	data_[row][col] = value;
}

bool MatrixArea::getValue(const uint32_t& row, const uint32_t& col) const
{
	return data_[row][col];
}

void MatrixArea::setCenter(const uint32_t& y, const uint32_t& x)
{
	centerX = x;
	centerY = y;
}

void MatrixArea::getCenter(uint32_t& y, uint32_t& x) const
{
	x = centerX;
	y = centerY;
}

const size_t& MatrixArea::getRows() const
{
	return rows;
}

const size_t& MatrixArea::getCols() const
{
	return cols;
}

const bool* MatrixArea::operator[](const uint32_t& i) const
{
	return data_[i];
}

bool* MatrixArea::operator[](const uint32_t& i)
{
	return data_[i];
}

//**********************************************************
AreaCombat::AreaCombat()
{
	hasExtArea = false;
}

AreaCombat::~AreaCombat()
{
	clear();
}

AreaCombat::AreaCombat(const AreaCombat& rhs)
{
	hasExtArea = rhs.hasExtArea;

	for (AreaCombatMap::const_iterator it = rhs.areas.begin(); it != rhs.areas.end(); ++it)
	{
		areas[it->first] = new MatrixArea(*it->second);
	}
}

void AreaCombat::clear()
{
	for (AreaCombatMap::iterator it = areas.begin(); it != areas.end(); ++it)
	{
		delete it->second;
	}

	areas.clear();
}

bool AreaCombat::getList(const Position& centerPos, const Position& targetPos, std::list<Tile*>& list) const
{
	Tile* tile = g_game.getTile(targetPos.x, targetPos.y, targetPos.z);
	const MatrixArea* area = getArea(centerPos, targetPos);

	if (!area)
	{
		return false;
	}

	int32_t tmpPosX = targetPos.x;
	int32_t tmpPosY = targetPos.y;
	int32_t tmpPosZ = targetPos.z;
	size_t cols = area->getCols();
	size_t rows = area->getRows();
	uint32_t centerY, centerX;
	area->getCenter(centerY, centerX);
	tmpPosX -= centerX;
	tmpPosY -= centerY;

	for (size_t y = 0; y < rows; ++y)
	{
		for (size_t x = 0; x < cols; ++x)
		{
			if (area->getValue(y, x) != 0)
			{
				if (tmpPosX >= 0 && tmpPosX < 0xFFFF &&
				        tmpPosY >= 0 && tmpPosY < 0xFFFF &&
				        tmpPosZ >= 0 && tmpPosZ < MAP_MAX_LAYERS)
				{
					if (g_game.isSightClear(targetPos, Position(tmpPosX, tmpPosY, tmpPosZ), true))
					{
						tile = g_game.getTile(tmpPosX, tmpPosY, tmpPosZ);

						if (!tile)
						{
							// This tile will never have anything on it
							tile = new StaticTile(tmpPosX, tmpPosY, tmpPosZ);
							g_game.setTile(tile);
						}

						list.push_back(tile);
					}
				}
			}

			tmpPosX += 1;
		}

		tmpPosX -= cols;
		tmpPosY += 1;
	}

	return true;
}

void AreaCombat::copyArea(const MatrixArea* input, MatrixArea* output, const MatrixOperation_t& op) const
{
	uint32_t centerY, centerX;
	input->getCenter(centerY, centerX);

	if (op == MATRIXOPERATION_COPY)
	{
		for (unsigned int y = 0; y < input->getRows(); ++y)
		{
			for (unsigned int x = 0; x < input->getCols(); ++x)
			{
				(*output)[y][x] = (*input)[y][x];
			}
		}

		output->setCenter(centerY, centerX);
	}
	else if (op == MATRIXOPERATION_MIRROR)
	{
		for (unsigned int y = 0; y < input->getRows(); ++y)
		{
			int rx = 0;

			for (int x = input->getCols() - 1; x >= 0; --x)
			{
				(*output)[y][rx++] = (*input)[y][x];
			}
		}

		output->setCenter(centerY, (input->getRows() - 1) - centerX);
	}
	else if (op == MATRIXOPERATION_FLIP)
	{
		for (unsigned int x = 0; x < input->getCols(); ++x)
		{
			int ry = 0;

			for (int y = input->getRows() - 1; y >= 0; --y)
			{
				(*output)[ry++][x] = (*input)[y][x];
			}
		}

		output->setCenter((input->getCols() - 1) - centerY, centerX);
	}
	//rotation
	else
	{
		uint32_t centerX, centerY;
		input->getCenter(centerY, centerX);
		int32_t rotateCenterX = (output->getCols() / 2) - 1;
		int32_t rotateCenterY = (output->getRows() / 2) - 1;
		int32_t angle = 0;

		switch (op)
		{
			case MATRIXOPERATION_ROTATE90:
				angle = 90;
				break;
			case MATRIXOPERATION_ROTATE180:
				angle = 180;
				break;
			case MATRIXOPERATION_ROTATE270:
				angle = 270;
				break;
			default:
				angle = 0;
				break;
		}

		double angleRad = 3.1416 * angle / 180.0;
		float a = std::cos(angleRad);
		float b = -std::sin(angleRad);
		float c = std::sin(angleRad);
		float d = std::cos(angleRad);

		for (int32_t x = 0; x < (int32_t)input->getCols(); ++x)
		{
			for (int32_t y = 0; y < (int32_t)input->getRows(); ++y)
			{
				//calculate new coordinates using rotation center
				int32_t newX = x - centerX;
				int32_t newY = y - centerY;
				//perform rotation
				int32_t rotatedX = round(newX * a + newY * b);
				int32_t rotatedY = round(newX * c + newY * d);
				//write in the output matrix using rotated coordinates
				(*output)[rotatedY + rotateCenterY][rotatedX + rotateCenterX] = (*input)[y][x];
			}
		}

		output->setCenter(rotateCenterY, rotateCenterX);
	}
}

MatrixArea* AreaCombat::getArea(const Position& centerPos, const Position& targetPos) const
{
	int32_t dx = targetPos.x - centerPos.x;
	int32_t dy = targetPos.y - centerPos.y;
	Direction dir = NORTH;

	if (dx < 0)
	{
		dir = WEST;
	}
	else if (dx > 0)
	{
		dir = EAST;
	}
	else if (dy < 0)
	{
		dir = NORTH;
	}
	else
	{
		dir = SOUTH;
	}

	if (hasExtArea)
	{
		if (dx < 0 && dy < 0)
		{
			dir = NORTHWEST;
		}
		else if (dx > 0 && dy < 0)
		{
			dir = NORTHEAST;
		}
		else if (dx < 0 && dy > 0)
		{
			dir = SOUTHWEST;
		}
		else if (dx > 0 && dy > 0)
		{
			dir = SOUTHEAST;
		}
	}

	AreaCombatMap::const_iterator it = areas.find(dir);

	if (it != areas.end())
	{
		return it->second;
	}

	return NULL;
}

int32_t AreaCombat::round(const float& v) const
{
	int32_t t = (int32_t)std::floor(v);

	if ((v - t) > 0.5)
	{
		return t + 1;
	}

	return t;
}

MatrixArea* AreaCombat::createArea(const std::list<uint32_t>& list, const uint32_t& rows)
{
	unsigned int cols = list.size() / rows;
	MatrixArea* area = new MatrixArea(rows, cols);
	uint32_t x = 0;
	uint32_t y = 0;

	for (std::list<uint32_t>::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if (*it == 1 || *it == 3)
		{
			area->setValue(y, x, true);
		}

		if (*it == 2 || *it == 3)
		{
			area->setCenter(y, x);
		}

		++x;

		if (cols == x)
		{
			x = 0;
			++y;
		}
	}

	return area;
}

void AreaCombat::setupArea(const std::list<uint32_t>& list, const uint32_t& rows)
{
	MatrixArea* area = createArea(list, rows);
	//NORTH
	areas[NORTH] = area;
	uint32_t maxOutput = std::max(area->getCols(), area->getRows()) * 2;
	//SOUTH
	MatrixArea* southArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, southArea, MATRIXOPERATION_ROTATE180);
	areas[SOUTH] = southArea;
	//EAST
	MatrixArea* eastArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, eastArea, MATRIXOPERATION_ROTATE90);
	areas[EAST] = eastArea;
	//WEST
	MatrixArea* westArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, westArea, MATRIXOPERATION_ROTATE270);
	areas[WEST] = westArea;
}

void AreaCombat::setupArea(const int32_t& length, const int32_t& spread)
{
	std::list<uint32_t> list;
	uint32_t rows = length;
	int32_t cols = 1;

	if (spread != 0)
	{
		cols = ((length - length % spread) / spread) * 2 + 1;
	}

	int32_t colSpread = cols;

	for (uint32_t y = 1; y <= rows; ++y)
	{
		int32_t mincol = cols - colSpread + 1;
		int32_t maxcol = cols - (cols - colSpread);

		for (int32_t x = 1; x <= cols; ++x)
		{
			if (y == rows && x == ((cols - cols % 2) / 2) + 1)
			{
				list.push_back(3);
			}
			else if (x >= mincol && x <= maxcol)
			{
				list.push_back(1);
			}
			else
			{
				list.push_back(0);
			}
		}

		if (spread > 0 && y % spread == 0)
		{
			--colSpread;
		}
	}

	setupArea(list, rows);
}

void AreaCombat::setupArea(const int32_t& radius)
{
	static const int32_t area[13][13] =
	{
		{0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 8, 8, 7, 8, 8, 0, 0, 0, 0},
		{0, 0, 0, 8, 7, 6, 6, 6, 7, 8, 0, 0, 0},
		{0, 0, 8, 7, 6, 5, 5, 5, 6, 7, 8, 0, 0},
		{0, 8, 7, 6, 5, 4, 4, 4, 5, 6, 7, 8, 0},
		{0, 8, 6, 5, 4, 3, 2, 3, 4, 5, 6, 8, 0},
		{8, 7, 6, 5, 4, 2, 1, 2, 4, 5, 6, 7, 8},
		{0, 8, 6, 5, 4, 3, 2, 3, 4, 5, 6, 8, 0},
		{0, 8, 7, 6, 5, 4, 4, 4, 5, 6, 7, 8, 0},
		{0, 0, 8, 7, 6, 5, 5, 5, 6, 7, 8, 0, 0},
		{0, 0, 0, 8, 7, 6, 6, 6, 7, 8, 0, 0, 0},
		{0, 0, 0, 0, 8, 8, 7, 8, 8, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0}
	};
	std::list<uint32_t> list;

	for (int32_t y = 0; y < 13; ++y)
	{
		for (int32_t x = 0; x < 13; ++x)
		{
			if (area[y][x] == 1)
			{
				list.push_back(3);
			}
			else if (area[y][x] > 0 && area[y][x] <= radius)
			{
				list.push_back(1);
			}
			else
			{
				list.push_back(0);
			}
		}
	}

	setupArea(list, 13);
}

void AreaCombat::setupExtArea(const std::list<uint32_t>& list, const uint32_t& rows)
{
	if (list.empty())
	{
		return;
	}

	hasExtArea = true;
	MatrixArea* area = createArea(list, rows);
	//NORTH-WEST
	areas[NORTHWEST] = area;
	uint32_t maxOutput = std::max(area->getCols(), area->getRows()) * 2;
	//NORTH-EAST
	MatrixArea* neArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, neArea, MATRIXOPERATION_MIRROR);
	areas[NORTHEAST] = neArea;
	//SOUTH-WEST
	MatrixArea* swArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, swArea, MATRIXOPERATION_FLIP);
	areas[SOUTHWEST] = swArea;
	//SOUTH-EAST
	MatrixArea* seArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(swArea, seArea, MATRIXOPERATION_MIRROR);
	areas[SOUTHEAST] = seArea;
}

//**********************************************************
MagicField::MagicField(const uint16_t& _type)
	: Item(_type)
{
	createTime = OTSYS_TIME();
}

MagicField::~MagicField()
{
	// Virtual Destructor
}

MagicField* MagicField::getMagicField()
{
	return this;
}

const MagicField* MagicField::getMagicField() const
{
	return this;
}

bool MagicField::isBlocking(const Creature* creature) const
{
	if (id == ITEM_MAGICWALL_SAFE || id == ITEM_WILDGROWTH_SAFE)
	{
		if (creature->getPlayer() && creature->getPlayer()->hasSomeInvisibilityFlag())
		{
			return false;
		}

		uint32_t ownerId = getOwner();

		if (ownerId != 0)
		{
			Creature* owner = g_game.getCreatureByID(ownerId);

			if (owner)
			{
				return !creature->canWalkthrough(owner);
			}
		}

		return true;
	}

	return Item::isBlocking(creature);
}

bool MagicField::isReplaceable() const
{
	return Item::items[getID()].replaceable;
}

CombatType_t MagicField::getCombatType() const
{
	const ItemType& it = items[getID()];
	return it.combatType;
}

bool MagicField::canOwnerHarm(const Creature* target) const
{
	uint32_t ownerId = getOwner();

	if (ownerId != 0)
	{
		Creature* owner = g_game.getCreatureByID(ownerId);

		if (owner && owner != target)
		{
			return (Combat::canDoCombat(owner, target) == RET_NOERROR);
		}
	}

	return true;
}

void MagicField::onStepInField(Creature* creature, bool purposeful/*= true*/)
{
	//remove magic walls/wild growth
	if (id == ITEM_MAGICWALL_SAFE || id == ITEM_WILDGROWTH_SAFE ||
	        id == ITEM_MAGICWALL || id == ITEM_WILDGROWTH)
	{
		bool isBlockingVar = isBlocking(creature);
		bool config = g_config.getBoolean(ConfigManager::MW_DISAPPEAR_ON_WALK);

		if (isBlockingVar || (!isBlockingVar && config))
		{
			g_game.internalRemoveItem(this, 1);
		}
	}
	else
	{
		const ItemType& it = items[getID()];

		if (it.condition)
		{
			if (canOwnerHarm(creature))
			{
				Condition* conditionCopy = it.condition->clone();
				uint32_t ownerId = getOwner();

				if (ownerId != 0 && purposeful)
				{
					Creature* owner = g_game.getCreatureByID(ownerId);

					if (owner && (owner != creature))
					{
						if ((OTSYS_TIME() - createTime <= g_config.getNumber(ConfigManager::FIELD_OWNERSHIP_DURATION)) ||
						        owner->hasBeenAttacked(ownerId))
						{
							conditionCopy->setParam(CONDITIONPARAM_OWNER, ownerId);
						}
					}
				}

				if (Player* player = creature->getPlayer())
				{
					if (ConditionDamage* conditionDamage = static_cast<ConditionDamage*>(conditionCopy))
					{
						Item* tmpItem = NULL;

						for (int32_t i = SLOT_FIRST; i <= SLOT_LAST; i++)
						{
							if ((tmpItem = player->getInventoryItem((slots_t)i)))
							{
								if (tmpItem->getWieldPosition() != i)
								{
									continue;
								}

								const ItemType& it = items[tmpItem->getID()];

								std::map<uint16_t, int16_t>::const_iterator id = it.abilities.absorbFieldDamage.find(getID());

								if (id != it.abilities.absorbFieldDamage.end())
								{
									int32_t index = 0, length = conditionDamage->getLength();
									std::list<IntervalInfo> damageList;

									for (; index < length; index++)
									{
										IntervalInfo info = conditionDamage->popFrontDamage();
										info.value = (int32_t)std::floor((double)info.value * (100 - id->second) / 100.);
										damageList.push_back(info);
										conditionDamage->setTicks(conditionDamage->getTicks() - info.interval);
									}

									index = 0;
									length -= it.abilities.conditionCount;
									conditionDamage->clearDamageList();

									for (std::list<IntervalInfo>::iterator itt = damageList.begin(); itt != damageList.end(); ++itt)
									{
										conditionDamage->addDamage(1, itt->interval, itt->value);

										if (++index == length)
										{
											break;
										}
									}
								}
							}
						}

						if (conditionDamage->getTotalDamage() > 0)
						{
							conditionDamage->setParam(CONDITIONPARAM_FORCEUPDATE, true);
						}

						creature->addCondition(conditionDamage);
						return;
					}
				}

				creature->addCondition(conditionCopy);
			}
		}
	}
}

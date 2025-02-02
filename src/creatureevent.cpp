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

#include "creatureevent.h"
#include "tools.h"
#include "player.h"
#include <sstream>


CreatureEvent::CreatureEvent(LuaScriptInterface* _interface)
	: Event(_interface)
{
	m_type = CREATURE_EVENT_NONE;
	m_isLoaded = false;
}

CreatureEvent::~CreatureEvent()
{
	// Virtual Destructor
}

bool CreatureEvent::configureEvent(xmlNodePtr p)
{
	std::string str;

	//Name that will be used in monster xml files and
	// lua function to register events to reference this event
	if (readXMLString(p, "name", str))
	{
		m_eventName = str;
	}
	else
	{
		std::cout << "Error: [CreatureEvent::configureEvent] No name for creature event." << std::endl;
		return false;
	}

	if (readXMLString(p, "type", str))
	{
		if (asLowerCaseString(str) == "login")
		{
			m_type = CREATURE_EVENT_LOGIN;
		}
		else if (asLowerCaseString(str) == "logout")
		{
			m_type = CREATURE_EVENT_LOGOUT;
		}
		else if (asLowerCaseString(str) == "die")
		{
			m_type = CREATURE_EVENT_DIE;
		}
		else if (asLowerCaseString(str) == "kill")
		{
			m_type = CREATURE_EVENT_KILL;
		}
		else if (asLowerCaseString(str) == "advance")
		{
			m_type = CREATURE_EVENT_ADVANCE;
		}
		else if (asLowerCaseString(str) == "look")
		{
			m_type = CREATURE_EVENT_LOOK;
		}
		else
		{
			std::cout << "Error: [CreatureEvent::configureEvent] No valid type for creature event." << str << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << "Error: [CreatureEvent::configureEvent] No type for creature event."  << std::endl;
		return false;
	}

	m_isLoaded = true;
	return true;
}

const CreatureEventType_t& CreatureEvent::getEventType() const
{
	return m_type;
}

const std::string& CreatureEvent::getName() const
{
	return m_eventName;
}

bool CreatureEvent::isLoaded() const
{
	return m_isLoaded;
}

const std::string& CreatureEvent::getScriptEventName() const
{
	static const std::string EVENT_NAME[] =
	{
		"",
		"onLogin",
		"onLogout",
		"onDie",
		"onKill",
		"onAdvance",
		"onLook"
	};
	return EVENT_NAME[m_type];
}

void CreatureEvent::copyEvent(CreatureEvent* creatureEvent)
{
	m_scriptId = creatureEvent->m_scriptId;
	m_scriptInterface = creatureEvent->m_scriptInterface;
	m_scripted = creatureEvent->m_scripted;
	m_isLoaded = creatureEvent->m_isLoaded;
}

void CreatureEvent::clearEvent()
{
	m_scriptId = 0;
	m_scriptInterface = NULL;
	m_scripted = false;
	m_isLoaded = false;
}

bool CreatureEvent::executeOnLogin(Player* player)
{
	//onLogin(cid)
	if (m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << player->getName();
		env->setEventDesc(desc.str());
#endif
		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(player->getPosition());
		uint32_t cid = env->addThing(player);
		lua_State* L = m_scriptInterface->getLuaState();
		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		bool result = m_scriptInterface->callFunction(1);
		m_scriptInterface->releaseScriptEnv();
		return result;
	}
	else
	{
		std::cout << "[Error] Call stack overflow. CreatureEvent::executeOnLogin" << std::endl;
		return 0;
	}
}

bool CreatureEvent::executeOnLogout(Player* player)
{
	//onLogout(cid)
	if (m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << player->getName();
		env->setEventDesc(desc.str());
#endif
		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(player->getPosition());
		uint32_t cid = env->addThing(player);
		lua_State* L = m_scriptInterface->getLuaState();
		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		bool result = m_scriptInterface->callFunction(1);
		m_scriptInterface->releaseScriptEnv();
		return result;
	}
	else
	{
		std::cout << "[Error] Call stack overflow. CreatureEvent::executeOnLogout" << std::endl;
		return 0;
	}
}

void CreatureEvent::executeOnDie(Creature* creature, Item* corpse)
{
	//onDie(cid, corpse)
	if (m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << creature->getName();
		env->setEventDesc(desc.str());
#endif
		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(creature->getPosition());
		uint32_t cid = env->addThing(creature);
		uint32_t corpseid = env->addThing(corpse);
		lua_State* L = m_scriptInterface->getLuaState();
		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		lua_pushnumber(L, corpseid);
		m_scriptInterface->callFunction(2, false);
		m_scriptInterface->releaseScriptEnv();
	}
	else
	{
		std::cout << "[Error] Call stack overflow. CreatureEvent::executeOnDie" << std::endl;
	}
}

void CreatureEvent::executeOnKill(Creature* creature, Creature* target, bool lastHit)
{
	//onKill(cid, target, lasthit)
	if (m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << creature->getName();
		env->setEventDesc(desc.str());
#endif
		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(creature->getPosition());
		uint32_t cid = env->addThing(creature);
		uint32_t targetId = env->addThing(target);
		lua_State* L = m_scriptInterface->getLuaState();
		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		lua_pushnumber(L, targetId);
		lua_pushboolean(L, (lastHit ? true : false));
		m_scriptInterface->callFunction(3, false);
		m_scriptInterface->releaseScriptEnv();
	}
	else
	{
		std::cout << "[Error] __ENABLE_SERVER_DIAGNOSTIC__Call stack overflow. CreatureEvent::executeOnKill" << std::endl;
	}
}

void CreatureEvent::executeOnAdvance(Player* player, levelTypes_t type, uint32_t oldLevel, uint32_t newLevel)
{
	//onAdvance(cid, type, oldlevel, newlevel)
	if (m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << player->getName();
		env->setEventDesc(desc.str());
#endif
		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(player->getPosition());
		uint32_t cid = env->addThing(player);
		lua_State* L = m_scriptInterface->getLuaState();
		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		lua_pushnumber(L, (uint32_t)type);
		lua_pushnumber(L, oldLevel);
		lua_pushnumber(L, newLevel);
		m_scriptInterface->callFunction(4, false);
		m_scriptInterface->releaseScriptEnv();
	}
	else
	{
		std::cout << "[Error] Call stack overflow. CreatureEvent::executeOnAdvance" << std::endl;
	}
}

bool CreatureEvent::executeOnLook(Player* player, Thing* target, uint16_t itemId)
{
	//onLook(cid, thing, itemId)
	if (m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << player->getName();
		env->setEventDesc(desc.str());
#endif
		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(player->getPosition());
		lua_State* L = m_scriptInterface->getLuaState();
		m_scriptInterface->pushFunction(m_scriptId);
		uint32_t cid = env->addThing(player);
		lua_pushnumber(L, cid);
		uint32_t target_id = 0;

		if (target)
		{
			if (target->getCreature())
			{
				target_id = env->addThing(target->getCreature());
			}
			else if (target->getItem())
			{
				target_id = env->addThing(target->getItem());
			}
			else
			{
				target = NULL;
			}
		}

		if (target)
		{
			LuaScriptInterface::pushThing(L, target, target_id);
		}
		else
		{
			lua_pushnil(L);
		}

		lua_pushnumber(L, itemId);
		bool result = m_scriptInterface->callFunction(3);
		m_scriptInterface->releaseScriptEnv();
		return result;
	}
	else
	{
		std::cout << "[Error] Call stack overflow. CreatureEvent::executeOnLook" << std::endl;
		return 0;
	}
}

CreatureEvents::CreatureEvents() :
	m_scriptInterface("CreatureScript Interface")
{
	m_scriptInterface.initState();
}

CreatureEvents::~CreatureEvents()
{
	CreatureEventList::iterator it;

	for (it = m_creatureEvents.begin(); it != m_creatureEvents.end(); ++it)
	{
		delete it->second;
	}
}

void CreatureEvents::clear()
{
	//clear all events
	CreatureEventList::iterator it;

	for (it = m_creatureEvents.begin(); it != m_creatureEvents.end(); ++it)
	{
		it->second->clearEvent();
	}

	//clear lua state
	m_scriptInterface.reInitState();
}

LuaScriptInterface& CreatureEvents::getScriptInterface()
{
	return m_scriptInterface;
}

const std::string& CreatureEvents::getScriptBaseName() const
{
	static const std::string BASE_NAME = "creaturescripts";
	return BASE_NAME;
}

Event* CreatureEvents::getEvent(const std::string& nodeName)
{
	if (asLowerCaseString(nodeName) == "event")
	{
		return new CreatureEvent(&m_scriptInterface);
	}

	return NULL;
}

bool CreatureEvents::registerEvent(Event* event, xmlNodePtr p)
{
	CreatureEvent* creatureEvent = dynamic_cast<CreatureEvent*>(event);

	if (!creatureEvent)
	{
		return false;
	}

	switch (creatureEvent->getEventType())
	{
		case CREATURE_EVENT_NONE:
			std::cout << "Error: [CreatureEvents::registerEvent] Trying to register event without type!." << std::endl;
			return false;
			break;
			// events are stored in a std::map
		default:
		{
			CreatureEvent* oldEvent = getEventByName(creatureEvent->getName(), false);

			if (oldEvent)
			{
				// if there was an event with the same that is not loaded
				// (happens when realoading), it is reused
				if (!oldEvent->isLoaded() &&
				        oldEvent->getEventType() == creatureEvent->getEventType())
				{
					oldEvent->copyEvent(creatureEvent);
				}

				return false;
			}
			else
			{
				//if not, register it normally
				m_creatureEvents[creatureEvent->getName()] = creatureEvent;
				return true;
			}
		}
	}
}

CreatureEvent* CreatureEvents::getEventByName(const std::string& name, bool forceLoaded /*= true*/)
{
	CreatureEventList::iterator it = m_creatureEvents.find(name);

	if (it != m_creatureEvents.end())
	{
		// After reloading, a creature can have script that was not
		// loaded again, if is the case, return NULL
		if (!forceLoaded || it->second->isLoaded())
		{
			return it->second;
		}
	}

	return NULL;
}


bool CreatureEvents::playerLogIn(Player* player)
{
	for (CreatureEventList::iterator it = m_creatureEvents.begin(); it != m_creatureEvents.end(); ++it)
	{
		if (it->second->getEventType() == CREATURE_EVENT_LOGIN)
		{
			if (!it->second->executeOnLogin(player))
			{
				return false;
			}
		}
	}

	return true;
}

bool CreatureEvents::playerLogOut(Player* player)
{
	for (CreatureEventList::iterator it = m_creatureEvents.begin(); it != m_creatureEvents.end(); ++it)
	{
		if (it->second->getEventType() == CREATURE_EVENT_LOGOUT)
		{
			if (!it->second->executeOnLogout(player))
			{
				return false;
			}
		}
	}

	return true;
}

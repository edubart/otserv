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

#ifndef __OTSERV_CREATUREEVENT_H__
#define __OTSERV_CREATUREEVENT_H__

#include "definitions.h"
#include "enums.h"
#include "luascript.h"
#include "baseevents.h"

enum CreatureEventType_t
{
	CREATURE_EVENT_NONE,
	CREATURE_EVENT_LOGIN,
	CREATURE_EVENT_LOGOUT,
	CREATURE_EVENT_DIE,
	CREATURE_EVENT_KILL,
	CREATURE_EVENT_ADVANCE,
	CREATURE_EVENT_LOOK
};

class CreatureEvent : public Event
{
public:
	CreatureEvent(LuaScriptInterface* _interface);
	virtual ~CreatureEvent();

	virtual bool configureEvent(xmlNodePtr p);

	const CreatureEventType_t& getEventType() const;
	const std::string& getName() const;
	bool isLoaded() const;

	void clearEvent();
	void copyEvent(CreatureEvent* creatureEvent);

	//scripting
	bool executeOnLogin(Player* player);
	bool executeOnLogout(Player* player);
	void executeOnDie(Creature* creature, Item* corpse);
	void executeOnKill(Creature* creature, Creature* target, bool lastHit);
	void executeOnAdvance(Player* player, levelTypes_t type, uint32_t oldLevel, uint32_t newLevel);
	bool executeOnLook(Player* player, Thing* target, uint16_t itemId);

protected:
	virtual const std::string& getScriptEventName() const;

	std::string m_eventName;
	CreatureEventType_t m_type;
	bool m_isLoaded;
};

class CreatureEvents : public BaseEvents
{
public:
	CreatureEvents();
	virtual ~CreatureEvents();

	// global events
	bool playerLogIn(Player* player);
	bool playerLogOut(Player* player);

	CreatureEvent* getEventByName(const std::string& name, bool forceLoaded = true);

protected:

	virtual LuaScriptInterface& getScriptInterface();
	virtual const std::string& getScriptBaseName() const;
	virtual Event* getEvent(const std::string& nodeName);
	virtual bool registerEvent(Event* event, xmlNodePtr p);
	virtual void clear();

	//events
	typedef std::map<std::string, CreatureEvent*> CreatureEventList;
	CreatureEventList m_creatureEvents;

	LuaScriptInterface m_scriptInterface;
};

#endif // __OTSERV_CREATUREEVENT_H__

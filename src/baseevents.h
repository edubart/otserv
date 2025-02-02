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

#ifndef __OTSERV_BASEEVENTS_H__
#define __OTSERV_BASEEVENTS_H__

#include "definitions.h"
#include "luascript.h"
#include <libxml/parser.h>

class Event
{
public:
	Event(LuaScriptInterface* _interface);
	virtual ~Event();

	virtual bool configureEvent(xmlNodePtr p) = 0;

	bool checkScript(const std::string& datadir, const std::string& scriptsName, const std::string& scriptFile);
	virtual bool loadScript(const std::string& scriptFile, bool reserveEnviroment = true);
	virtual bool loadFunction(const std::string& functionName);
	const int32_t& getScriptId() const;
	LuaScriptInterface* getScriptInterface();

protected:
	virtual const std::string& getScriptEventName() const = 0;

	int32_t m_scriptId;
	LuaScriptInterface* m_scriptInterface;

	bool m_scripted;
};

class BaseEvents
{
public:
	BaseEvents();
	virtual ~BaseEvents();

	bool loadFromXml(const std::string& datadir);
	bool reload();
	bool isLoaded() const;

protected:
	virtual LuaScriptInterface& getScriptInterface() = 0;
	virtual const std::string& getScriptBaseName() const = 0;
	virtual Event* getEvent(const std::string& nodeName) = 0;
	virtual bool registerEvent(Event* event, xmlNodePtr p) = 0;
	virtual void clear() = 0;

	bool m_loaded;
	std::string m_datadir;
};

class CallBack
{
public:
	CallBack();
	virtual ~CallBack();

	bool loadCallBack(LuaScriptInterface* _interface, const std::string& name);

protected:
	int32_t m_scriptId;
	LuaScriptInterface* m_scriptInterface;

	bool m_loaded;

	std::string m_callbackName;
};

#endif // __OTSERV_BASEEVENTS_H__

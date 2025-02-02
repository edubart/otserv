//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Quests
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

#include "quests.h"
#include "tools.h"

MissionState::MissionState(const std::string& _description, const uint32_t& _missionID)
	: description(_description)
	, missionID(missionID)
{}

const uint32_t& MissionState::getMissionID() const
{
	return missionID;
}

const std::string& MissionState::getMissionDescription() const
{
	return description;
}

Mission::Mission(const std::string& _missionName, const uint32_t& _storageID,
                 const uint32_t& _startValue, const int32_t& _endValue)
	: mainState(NULL)
	, missionName(_missionName)
	, storageID(_storageID)
	, startValue(_startValue)
	, endValue(_endValue)
{}

Mission::~Mission()
{
	for (uint32_t it = 0; it != state.size(); ++it)
	{
		delete state[it];
	}
}

std::string Mission::getDescription(Player* player)
{
	int32_t value;
	MissionState* lastState = 0;
	player->getStorageValue(storageID, value);

	if (mainState)
	{
		lastState = mainState;
	}
	else
	{
		StateList::iterator it;

		for (int32_t i = startValue; i <= value; ++i)
		{
			it = state.find(i);

			if (it != state.end())
			{
				lastState = it->second;
			}
		}
	}

	if (!lastState)
	{
		return "An error has occurred, please contact a gamemaster.";
	}

	std::stringstream s;
	s << value;
	std::string desc = lastState->getMissionDescription();
	replaceString(desc, "|STATE|", s.str());
	return desc;
}


bool Mission::isStarted(Player* player) const
{
	if (player)
	{
		uint32_t value = 0;
		player->getStorageValue(storageID, (int32_t&)value);

		if (value >= startValue && value <= endValue)
		{
			return true;
		}
	}

	return false;
}

bool Mission::isCompleted(Player* player) const
{
	if (player)
	{
		int32_t value = 0;
		player->getStorageValue(storageID, value);

		if (uint32_t(value) == endValue)
		{
			return true;
		}
	}

	return false;
}

std::string Mission::getName(Player* player)
{
	if (isCompleted(player))
	{
		return missionName + " (completed)";
	}

	return missionName;
}

Quest::Quest(const std::string& _name, const uint16_t& _id,
             const uint32_t& _startStorageID, const uint32_t& _startStorageValue)
	: name(_name)
	, id(_id)
	, startStorageID(_startStorageID)
	, startStorageValue(_startStorageValue)
{}

Quest::~Quest()
{
	MissionsList::iterator it;

	for (it = missions.begin(); it != missions.end(); ++it)
	{
		delete(*it);
	}
}

uint16_t Quest::getMissionsCount(Player* player) const
{
	uint16_t count = 0;

	for (MissionsList::const_iterator it = missions.begin(); it != missions.end(); ++it)
	{
		if ((*it)->isStarted(player))
		{
			count++;
		}
	}

	return count;
}

void Quest::addMission(Mission* mission)
{
	missions.push_back(mission);
}

MissionsList::const_iterator Quest::getFirstMission() const
{
	return missions.begin();
}

MissionsList::const_iterator Quest::getEndMission() const
{
	return missions.end();
}

bool Quest::isCompleted(Player* player)
{
	MissionsList::iterator it;

	for (it = missions.begin(); it != missions.end(); ++it)
	{
		if (!(*it)->isCompleted(player))
		{
			return false;
		}
	}

	return true;
}

bool Quest::isStarted(Player* player) const
{
	if (player)
	{
		uint32_t value = 0;
		player->getStorageValue(startStorageID, (int32_t&)value);

		if (value >= startStorageValue)
		{
			return true;
		}
	}

	return false;
}

const uint16_t& Quest::getID() const
{
	return id;
}

const std::string& Quest::getName() const
{
	return name;
}

Quests::Quests()
{
	//
}

Quests::~Quests()
{
	QuestsList::iterator it;

	for (it = quests.begin(); it != quests.end(); ++it)
	{
		delete(*it);
	}
}

Quests* Quests::getInstance()
{
	static Quests instance;
	return &instance;
}

QuestsList::const_iterator Quests::getFirstQuest() const
{
	return quests.begin();
}

QuestsList::const_iterator Quests::getEndQuest() const
{
	return quests.end();
}

bool Quests::reload()
{
	QuestsList::iterator it;

	for (it = quests.begin(); it != quests.end(); ++it)
	{
		delete(*it);
	}

	quests.clear();
	return loadFromXml(filename);
}

bool Quests::loadFromXml(const std::string& _filename)
{
	std::string filename = _filename;
	xmlDocPtr doc = xmlParseFile(filename.c_str());

	if (doc)
	{
		xmlNodePtr root, p;
		root = xmlDocGetRootElement(doc);

		if (xmlStrcmp(root->name, (const xmlChar*)"quests") == 0)
		{
			int32_t intValue;
			std::string strValue;
			uint16_t id = 0;
			p = root->children;

			while (p)
			{
				if (xmlStrcmp(p->name, (const xmlChar*)"quest") == 0)
				{
					std::string name;
					uint32_t startStorageID = 0, startStorageValue = 0;

					if (readXMLString(p, "name", strValue))
					{
						name = strValue;
					}

					if (readXMLInteger(p, "startstorageid", intValue))
					{
						startStorageID = intValue;
					}

					if (readXMLInteger(p, "startstoragevalue", intValue))
					{
						startStorageValue = intValue;
					}

					Quest* quest = new Quest(name, id, startStorageID, startStorageValue);
					xmlNodePtr tmpNode = p->children;

					while (tmpNode)
					{
						if (xmlStrcmp(tmpNode->name, (const xmlChar*)"mission") == 0)
						{
							std::string missionName, missionState;
							uint32_t storageID = 0, startValue = 0, endValue = 0;

							if (readXMLString(tmpNode, "name", strValue))
							{
								missionName = strValue;
							}

							if (readXMLInteger(tmpNode, "storageid", intValue))
							{
								storageID = intValue;
							}

							if (readXMLInteger(tmpNode, "startvalue", intValue))
							{
								startValue = intValue;
							}

							if (readXMLInteger(tmpNode, "endvalue", intValue))
							{
								endValue = intValue;
							}

							if (readXMLString(tmpNode, "description", strValue))
							{
								missionState = strValue;
							}

							Mission* mission = new Mission(missionName, storageID, startValue, endValue);

							if (missionState.empty())
							{
								// parse sub-states only if there's no main one
								xmlNodePtr tmpNode2 = tmpNode->children;

								while (tmpNode2)
								{
									if (xmlStrcmp(tmpNode2->name, (const xmlChar*)"missionstate") == 0)
									{
										std::string description;
										uint32_t missionID = 0;

										if (readXMLInteger(tmpNode2, "id", intValue))
										{
											missionID = intValue;
										}

										if (readXMLString(tmpNode2, "description", strValue))
										{
											description = strValue;
										}

										mission->state[missionID] = new MissionState(description, missionID);
									}

									tmpNode2 = tmpNode2->next;
								}
							}
							else
							{
								mission->mainState = new MissionState(missionState, 0);
							}

							quest->addMission(mission);
						}

						tmpNode = tmpNode->next;
					}

					quests.push_back(quest);
				}

				id++;
				p = p->next;
			}
		}

		xmlFreeDoc(doc);
		return true;
	}

	return false;
}

Quest* Quests::getQuestByID(const uint16_t& id)
{
	QuestsList::iterator it;

	for (it = quests.begin(); it != quests.end(); ++it)
	{
		if ((*it)->getID() == id)
		{
			return (*it);
		}
	}

	return NULL;
}

uint16_t Quests::getQuestsCount(Player* player)
{
	uint16_t count = 0;
	QuestsList::iterator it;

	for (it = quests.begin(); it != quests.end(); ++it)
	{
		if ((*it)->isStarted(player))
		{
			count++;
		}
	}

	return count;
}

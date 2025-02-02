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

#include "outfit.h"
#include "tools.h"
#include "player.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <iostream>

Outfit::Outfit()
	: outfitId(0)
	, lookType(0)
	, addons(0)
	, isPremium(false)
	, isDefault(false)
{}

Outfits::Outfits()
{
	//
}

Outfits* Outfits::getInstance()
{
	static Outfits instance;
	return &instance;
}

bool Outfits::loadFromXml(const std::string& datadir)
{
	m_datadir = datadir;
	std::string filename = datadir + "outfits.xml";
	xmlDocPtr doc = xmlParseFile(filename.c_str());

	if (doc)
	{
		xmlNodePtr root, p;
		root = xmlDocGetRootElement(doc);

		if (xmlStrcmp(root->name, (const xmlChar*)"outfits") != 0)
		{
			xmlFreeDoc(doc);
			std::cout << "Warning: outfits.xml not found, using defaults." << std::endl;
			return true;
		}

		p = root->children;

		while (p)
		{
			if (xmlStrcmp(p->name, (const xmlChar*)"outfit") == 0)
			{
				int32_t intValue;
				std::string strValue;

				if (readXMLInteger(p, "id", intValue))
				{
					Outfit outfit;
					outfit.outfitId = intValue;
					outfit.isDefault = true;

					if (readXMLInteger(p, "premium", intValue))
					{
						outfit.isPremium = (intValue == 1);
					}

					if (readXMLInteger(p, "default", intValue))
					{
						outfit.isDefault = (intValue == 1);
					}

					xmlNodePtr pchild = p->children;

					while (pchild)
					{
						if (xmlStrcmp(pchild->name, (const xmlChar*)"list") == 0)
						{
							if (readXMLInteger(pchild, "looktype", intValue))
							{
								outfit.lookType = intValue;
							}
							else
							{
								std::cout << "Missing looktype for an outfit." << std::endl;
								pchild = pchild->next;
								continue;
							}

							if (readXMLInteger(pchild, "addons", intValue))
							{
								outfit.addons = intValue;
							}

							if (readXMLString(pchild, "name", strValue))
							{
								outfit.name = strValue;
							}

							if (readXMLString(pchild, "type", strValue))
							{
								PlayerSex_t playersex = PLAYERSEX_LAST;

								if (asLowerCaseString(strValue) == "female")
								{
									playersex = PLAYERSEX_FEMALE;
								}
								else if (asLowerCaseString(strValue) == "male")
								{
									playersex = PLAYERSEX_MALE;
								}
								else if (asLowerCaseString(strValue) == "femalegm")
								{
									playersex = PLAYERSEX_FEMALE_GAMEMASTER;
								}
								else if (asLowerCaseString(strValue) == "malegm")
								{
									playersex = PLAYERSEX_MALE_GAMEMASTER;
								}
								else if (asLowerCaseString(strValue) == "femalecm")
								{
									playersex = PLAYERSEX_FEMALE_MANAGER;
								}
								else if (asLowerCaseString(strValue) == "malecm")
								{
									playersex = PLAYERSEX_MALE_MANAGER;
								}
								else if (asLowerCaseString(strValue) == "femalegod")
								{
									playersex = PLAYERSEX_FEMALE_GOD;
								}
								else if (asLowerCaseString(strValue) == "malegod")
								{
									playersex = PLAYERSEX_MALE_GOD;
								}
								else
								{
									std::cout << "Invalid player sex " << strValue << " for an outfit." << std::endl;
									pchild = pchild->next;
									continue;
								}

								allOutfits.push_back(outfit);
								outfitMaps[playersex][outfit.outfitId] = outfit;
							}
							else
							{
								std::cout << "Missing playersex for an outfit." << std::endl;
								pchild = pchild->next;
								continue;
							}
						}

						pchild = pchild->next;
					}
				}
				else
				{
					std::cout << "Missing outfit id." << std::endl;
				}
			}

			p = p->next;
		}

		xmlFreeDoc(doc);
	}

	return true;
}

bool Outfits::reload()
{
	allOutfits.clear();
	outfitMaps.clear();
	bool result = loadFromXml(m_datadir);

	if (result)
	{
		for (AutoList<Player>::listiterator it = Player::listPlayer.list.begin();
		        it != Player::listPlayer.list.end();
		        ++it)
		{
			it->second->setSex(it->second->getSex());
			it->second->setCurrentOutfit(it->second->getCurrentOutfit());
		}
	}

	return result;
}


const uint32_t& Outfits::getOutfitId(const uint32_t& lookType)
{
	static const uint32_t OUTFIT_NOT_FOUND = 0;

	for (OutfitList::iterator it = allOutfits.begin(); it != allOutfits.end(); ++it)
	{
		if (it->lookType == lookType)
		{
			return it->outfitId;
		}
	}

	return OUTFIT_NOT_FOUND;
}

bool Outfits::getOutfit(const uint32_t& lookType, Outfit& outfit)
{
	for (OutfitList::iterator it = allOutfits.begin(); it != allOutfits.end(); ++it)
	{
		if (it->lookType == lookType)
		{
			outfit = *it;
			return true;
		}
	}

	return false;
}

bool Outfits::getOutfit(const uint32_t& outfitId, const PlayerSex_t& sex, Outfit& outfit)
{
	OutfitMap map = getOutfits(sex);
	OutfitMap::iterator it = map.find(outfitId);

	if (it != map.end())
	{
		outfit = it->second;
		return true;
	}

	return false;
}

const OutfitMap& Outfits::getOutfits(const PlayerSex_t& playersex)
{
	return outfitMaps[playersex];
}

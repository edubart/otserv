//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Represents an item
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

#include "item.h"
#include "container.h"
#include "depot.h"
#include "teleport.h"
#include "trashholder.h"
#include "mailbox.h"
#include "house.h"
#include "game.h"
#include "luascript.h"
#include "actions.h"
#include "combat.h"
#include "weapons.h"
#include "beds.h"
#include <iostream>
#include <sstream>
#include <iomanip>

extern Game g_game;
extern Weapons* g_weapons;


ItemAttributes::ItemAttributes()
{
	m_attributes = 0;
	m_firstAttr = NULL;
}

ItemAttributes::ItemAttributes(const ItemAttributes& i)
{
	m_attributes = i.m_attributes;

	if (i.m_firstAttr)
	{
		m_firstAttr = new Attribute(*i.m_firstAttr);
	}
}

ItemAttributes::~ItemAttributes()
{
	if (m_firstAttr)
	{
		deleteAttrs(m_firstAttr);
	}
}

void ItemAttributes::setSpecialDescription(const std::string& desc)
{
	setStrAttr(ATTR_ITEM_DESC, desc);
}

void ItemAttributes::resetSpecialDescription()
{
	removeAttribute(ATTR_ITEM_DESC);
}

const std::string& ItemAttributes::getSpecialDescription() const
{
	return getStrAttr(ATTR_ITEM_DESC);
}

void ItemAttributes::setText(const std::string& text)
{
	setStrAttr(ATTR_ITEM_TEXT, text);
}

void ItemAttributes::resetText()
{
	removeAttribute(ATTR_ITEM_TEXT);
}

const std::string& ItemAttributes::getText() const
{
	return getStrAttr(ATTR_ITEM_TEXT);
}

void ItemAttributes::setWrittenDate(const time_t& n)
{
	setIntAttr(ATTR_ITEM_WRITTENDATE, (uint32_t)n);
}

void ItemAttributes::resetWrittenDate()
{
	removeAttribute(ATTR_ITEM_WRITTENDATE);
}

time_t ItemAttributes::getWrittenDate() const
{
	return (time_t)getIntAttr(ATTR_ITEM_WRITTENDATE);
}

void ItemAttributes::setWriter(const std::string& _writer)
{
	setStrAttr(ATTR_ITEM_WRITTENBY, _writer);
}

void ItemAttributes::resetWriter()
{
	removeAttribute(ATTR_ITEM_WRITTENBY);
}

const std::string& ItemAttributes::getWriter() const
{
	return getStrAttr(ATTR_ITEM_WRITTENBY);
}

void ItemAttributes::setActionId(const uint16_t& n)
{
	setIntAttr(ATTR_ITEM_ACTIONID, n > 100 ? n : 100);
}

uint16_t ItemAttributes::getActionId() const
{
	return (uint16_t)getIntAttr(ATTR_ITEM_ACTIONID);
}

void ItemAttributes::setUniqueId(const uint16_t& n)
{
	setIntAttr(ATTR_ITEM_UNIQUEID, n > 1000 ? n : 1000);
}

uint16_t ItemAttributes::getUniqueId() const
{
	return (uint16_t)getIntAttr(ATTR_ITEM_UNIQUEID);
}

void ItemAttributes::setCharges(const uint16_t& n)
{
	setIntAttr(ATTR_ITEM_CHARGES, n);
}

uint16_t ItemAttributes::getCharges() const
{
	return (uint16_t)getIntAttr(ATTR_ITEM_CHARGES);
}

void ItemAttributes::setFluidType(const uint16_t& n)
{
	setIntAttr(ATTR_ITEM_FLUIDTYPE, n);
}

uint16_t ItemAttributes::getFluidType() const
{
	return (uint16_t)getIntAttr(ATTR_ITEM_FLUIDTYPE);
}

void ItemAttributes::setOwner(const uint32_t& _owner)
{
	setIntAttr(ATTR_ITEM_OWNER, _owner);
}

uint32_t ItemAttributes::getOwner() const
{
	return (uint32_t)getIntAttr(ATTR_ITEM_OWNER);
}

void ItemAttributes::setCorpseOwner(const uint32_t& _corpseOwner)
{
	setIntAttr(ATTR_ITEM_CORPSEOWNER, _corpseOwner);
}

uint32_t ItemAttributes::getCorpseOwner()
{
	return (uint32_t)getIntAttr(ATTR_ITEM_CORPSEOWNER);
}

void ItemAttributes::setDuration(const int32_t& time)
{
	setIntAttr(ATTR_ITEM_DURATION, time);
}

void ItemAttributes::decreaseDuration(const int32_t& time)
{
	increaseIntAttr(ATTR_ITEM_DURATION, -time);
}

int32_t ItemAttributes::getDuration() const
{
	return (int32_t)getIntAttr(ATTR_ITEM_DURATION);
}

void ItemAttributes::setDecaying(const ItemDecayState_t& decayState)
{
	setIntAttr(ATTR_ITEM_DECAYING, decayState);
}

uint32_t ItemAttributes::getDecaying() const
{
	return (uint32_t)getIntAttr(ATTR_ITEM_DECAYING);
}

bool ItemAttributes::hasAttribute(const itemAttrTypes& type) const
{
	if (!validateIntAttrType(type))
	{
		return false;
	}

	Attribute* attr = getAttrConst(type);

	if (attr)
	{
		return true;
	}

	return false;
}

void ItemAttributes::removeAttribute(const itemAttrTypes& type)
{
	//check if we have it
	if ((type & m_attributes) != 0)
	{
		//go trough the linked list until find it
		Attribute* prevAttr = NULL;
		Attribute* curAttr = m_firstAttr;

		while (curAttr)
		{
			if (curAttr->type == type)
			{
				//found so remove it from the linked list
				if (prevAttr)
				{
					prevAttr->next = curAttr->next;
				}
				else
				{
					m_firstAttr = curAttr->next;
				}

				//remove it from flags
				m_attributes = m_attributes & ~type;

				//delete string if it is string type
				if (validateStrAttrType(type))
				{
					delete(std::string*)curAttr->value;
				}

				//finally delete the attribute and return
				delete curAttr;
				return;
			}

			//advance in the linked list
			prevAttr = curAttr;
			curAttr = curAttr->next;
		}
	}
}

const std::string& ItemAttributes::getStrAttr(const itemAttrTypes& type) const
{
	static const std::string EMPTY_STRING;

	if (!validateStrAttrType(type))
	{
		return EMPTY_STRING;
	}

	//this will *NOT* create the attribute if it does not exist
	Attribute* attr = getAttrConst(type);

	if (attr)
	{
		return *(std::string*)attr->value;
	}
	else
	{
		return EMPTY_STRING;
	}
}

void ItemAttributes::setStrAttr(const itemAttrTypes& type, const std::string& value)
{
	if (!validateStrAttrType(type))
	{
		return;
	}

	if (value.length() == 0)
	{
		return;
	}

	//this will create the attribute if it does not exist
	Attribute* attr = getAttr(type);

	if (attr)
	{
		//if has a previous value delete it
		if (attr->value)
		{
			delete(std::string*)attr->value;
		}

		//create the new value as string
		attr->value = (void*)new std::string(value);
	}
}

uint32_t ItemAttributes::getIntAttr(const itemAttrTypes& type) const
{
	if (!validateIntAttrType(type))
	{
		return 0;
	}

	Attribute* attr = getAttrConst(type);

	if (attr)
	{
		return static_cast<uint32_t>(0xFFFFFFFF & reinterpret_cast<ptrdiff_t>(attr->value));
	}
	else
	{
		return 0;
	}
}

void ItemAttributes::setIntAttr(const itemAttrTypes& type, const int32_t& value)
{
	if (!validateIntAttrType(type))
	{
		return;
	}

	Attribute* attr = getAttr(type);

	if (attr)
	{
		attr->value = reinterpret_cast<void*>(static_cast<ptrdiff_t>(value));
	}
}

void ItemAttributes::increaseIntAttr(const itemAttrTypes& type, const int32_t& value)
{
	if (!validateIntAttrType(type))
	{
		return;
	}

	Attribute* attr = getAttr(type);

	if (attr)
	{
		attr->value = reinterpret_cast<void*>(static_cast<ptrdiff_t>(static_cast<uint32_t>(0xFFFFFFFF & reinterpret_cast<ptrdiff_t>(attr->value)) + value));
	}
}

bool ItemAttributes::validateIntAttrType(const itemAttrTypes& type)
{
	//list of numeric type attributes
	switch (type)
	{
		case ATTR_ITEM_ACTIONID:
		case ATTR_ITEM_UNIQUEID:
		case ATTR_ITEM_OWNER:
		case ATTR_ITEM_DURATION:
		case ATTR_ITEM_DECAYING:
		case ATTR_ITEM_WRITTENDATE:
		case ATTR_ITEM_CORPSEOWNER:
		case ATTR_ITEM_CHARGES:
		case ATTR_ITEM_FLUIDTYPE:
		case ATTR_ITEM_DOORID:
			return true;
			break;
		default:
			return false;
			break;
	}

	return false;
}

bool ItemAttributes::validateStrAttrType(const itemAttrTypes& type)
{
	//list of text type attributes
	switch (type)
	{
		case ATTR_ITEM_DESC:
		case ATTR_ITEM_TEXT:
		case ATTR_ITEM_WRITTENBY:
			return true;
			break;
		default:
			return false;
			break;
	}

	return false;
}

void ItemAttributes::addAttr(Attribute* attr)
{
	//add an attribute to the linked list
	if (m_firstAttr)
	{
		//is not the first, so look for the end of the list
		Attribute* curAttr = m_firstAttr;

		while (curAttr->next)
		{
			curAttr = curAttr->next;
		}

		//and add it at the end
		curAttr->next = attr;
	}
	else
	{
		//is the first
		m_firstAttr = attr;
	}

	//add it to flags
	m_attributes = m_attributes | attr->type;
}

ItemAttributes::Attribute* ItemAttributes::getAttrConst(const itemAttrTypes& type) const
{
	//check flags
	if ((type & m_attributes) == 0)
	{
		return NULL;
	}

	//it is here, so search it in the linked list
	Attribute* curAttr = m_firstAttr;

	while (curAttr)
	{
		if (curAttr->type == type)
		{
			//found
			return curAttr;
		}

		curAttr = curAttr->next;
	}

	//not found?
	std::cout << "Warning: [ItemAttributes::getAttrConst] (type & m_attributes) != 0 but attribute not found" << std::endl;
	return NULL;
}

ItemAttributes::Attribute* ItemAttributes::getAttr(const itemAttrTypes& type)
{
	Attribute* curAttr;

	if ((type & m_attributes) == 0)
	{
		//if that type was not present add it
		curAttr = new Attribute(type);
		addAttr(curAttr);
		return curAttr;
	}
	else
	{
		//was present, search and return it
		curAttr = m_firstAttr;

		while (curAttr)
		{
			if (curAttr->type == type)
			{
				return curAttr;
			}

			curAttr = curAttr->next;
		}
	}

	std::cout << "Warning: [ItemAttributes::getAttr] (type & m_attributes) != 0 but attribute not found" << std::endl;
	curAttr = new Attribute(type);
	addAttr(curAttr);
	return curAttr;
}

void ItemAttributes::deleteAttrs(Attribute* attr)
{
	//deletes all attributes recursively
	if (attr)
	{
		//if is string type, delete the allocated string
		if (validateStrAttrType(attr->type))
		{
			delete(std::string*)attr->value;
		}

		Attribute* next_attr = attr->next;
		attr->next = NULL;

		//delete next while it was not NULL
		if (next_attr)
		{
			deleteAttrs(next_attr);
		}

		delete attr;
	}
}


Items Item::items;

Item* Item::CreateItem(const uint16_t& _type, const uint16_t& _count /*= 0*/)
{
	Item* newItem = NULL;
	const ItemType& it = Item::items[_type];

	if (it.group == ITEM_GROUP_DEPRECATED)
	{
#ifdef __DEBUG__
		std::cout << "Error: [Item::CreateItem] Item id " << it.id << " has been declared deprecated."  << std::endl;
#endif
		return NULL;
	}

	if (it.id != 0)
	{
		if (it.isDepot())
		{
			newItem = new Depot(_type);
		}
		else if (it.isContainer())
		{
			newItem = new Container(_type);
		}
		else if (it.isTeleport())
		{
			newItem = new Teleport(_type);
		}
		else if (it.isMagicField())
		{
			newItem = new MagicField(_type);
		}
		else if (it.isDoor())
		{
			newItem = new Door(_type);
		}
		else if (it.isTrashHolder())
		{
			newItem = new TrashHolder(_type, it.magicEffect);
		}
		else if (it.isMailbox())
		{
			newItem = new Mailbox(_type);
		}
		else if (it.isBed())
		{
			newItem = new BedItem(_type);
		}
		else
		{
			if (it.stackable && !_count)
			{
				newItem = new Item(_type, 1);
			}
			else
			{
				newItem = new Item(_type, _count);
			}
		}

		newItem->useThing2();
	}

	return newItem;
}

Item* Item::CreateItem(PropStream& propStream)
{
	uint16_t _id;

	if (!propStream.GET_UINT16(_id))
	{
		return NULL;
	}

	return Item::CreateItem(_id, 0);
}


bool Item::loadItem(xmlNodePtr node, Container* parent)
{
	if (xmlStrcmp(node->name, (const xmlChar*)"item") == 0)
	{
		int32_t intValue;
		std::string strValue;
		Item* item = NULL;

		if (readXMLInteger(node, "id", intValue))
		{
			item = Item::CreateItem(intValue);
		}

		if (!item)
		{
			return false;
		}

		//optional
		if (readXMLInteger(node, "subtype", intValue))
		{
			item->setSubType(intValue);
		}

		if (readXMLInteger(node, "actionid", intValue))
		{
			item->setActionId(intValue);
		}

		if (readXMLString(node, "text", strValue))
		{
			item->setText(strValue);
		}

		if (item->getContainer())
		{
			loadContainer(node, item->getContainer());
		}

		if (parent)
		{
			parent->addItem(item);
		}

		return true;
	}

	return false;
}

bool Item::loadContainer(xmlNodePtr parentNode, Container* parent)
{
	xmlNodePtr node = parentNode->children;

	while (node)
	{
		if (node->type != XML_ELEMENT_NODE)
		{
			node = node->next;
			continue;
		}

		if (xmlStrcmp(node->name, (const xmlChar*)"item") == 0)
		{
			if (!loadItem(node, parent))
			{
				return false;
			}
		}

		node = node->next;
	}

	return true;
}

Item::Item(const uint16_t& _type, const uint16_t& _count /*= 0*/)
	: ItemAttributes()
{
	id = _type;
	const ItemType& it = items[id];
	setItemCount(1);

	if (it.isFluidContainer() || it.isSplash())
	{
		setFluidType(_count);
	}
	else if (it.stackable)
	{
		if (_count != 0)
		{
			setItemCount(_count);
		}
		else if (it.charges != 0)
		{
			setItemCount(it.charges);
		}
	}
	else if (it.charges != 0)
	{
		//if count is higher than 1, override default charges
		if (_count > 1)
		{
			setCharges(_count);
		}
		else
		{
			setCharges(it.charges);
		}
	}

	setDefaultDuration();
}

Item::Item(const Item& i)
	: Thing()
	, ItemAttributes()
{
	//std::cout << "Item copy constructor " << this << std::endl;
	id = i.id;
	count = i.count;
	m_attributes = i.m_attributes;

	if (i.m_firstAttr)
	{
		m_firstAttr = new Attribute(*i.m_firstAttr);
	}
}

Item::~Item()
{
	// Virtual Destructor
}

Item* Item::clone() const
{
	Item* _item = Item::CreateItem(id, count);
	_item->m_attributes = m_attributes;

	if (m_firstAttr)
	{
		_item->m_firstAttr = new Attribute(*m_firstAttr);
	}

	return _item;
}

void Item::copyAttributes(Item* item)
{
	m_attributes = item->m_attributes;

	if (item->m_firstAttr)
	{
		m_firstAttr = new Attribute(*item->m_firstAttr);
	}

	removeAttribute(ATTR_ITEM_DECAYING);
	removeAttribute(ATTR_ITEM_DURATION);
}

Item* Item::getItem()
{
	return this;
}

const Item* Item::getItem() const
{
	return this;
}

Container* Item::getContainer()
{
	return NULL;
}

const Container* Item::getContainer() const
{
	return NULL;
}

Teleport* Item::getTeleport()
{
	return NULL;
}

const Teleport* Item::getTeleport() const
{
	return NULL;
}

TrashHolder* Item::getTrashHolder()
{
	return NULL;
}

const TrashHolder* Item::getTrashHolder() const
{
	return NULL;
}

Mailbox* Item::getMailbox()
{
	return NULL;
}

const Mailbox* Item::getMailbox() const
{
	return NULL;
}

Door* Item::getDoor()
{
	return NULL;
}

const Door* Item::getDoor() const
{
	return NULL;
}

MagicField* Item::getMagicField()
{
	return NULL;
}

const MagicField* Item::getMagicField() const
{
	return NULL;
}

BedItem* Item::getBed()
{
	return NULL;
}

const BedItem* Item::getBed() const
{
	return NULL;
}

void Item::setDefaultSubtype()
{
	const ItemType& it = items[id];
	setItemCount(1);

	if (it.charges != 0)
	{
		if (it.stackable)
		{
			setItemCount(it.charges);
		}
		else
		{
			setCharges(it.charges);
		}
	}
}

void Item::onRemoved()
{
	ScriptEnviroment::removeTempItem(this);

	if (getUniqueId() != 0)
	{
		ScriptEnviroment::removeUniqueThing(this);
	}
}

void Item::setID(const uint16_t& newid)
{
	const ItemType& prevIt = Item::items[id];
	id = newid;
	const ItemType& it = Item::items[newid];
	uint32_t newDuration = it.decayTime * 1000;

	if (newDuration == 0 && !it.stopTime && it.decayTo == -1)
	{
		removeAttribute(ATTR_ITEM_DECAYING);
		removeAttribute(ATTR_ITEM_DURATION);
	}

	if (hasAttribute(ATTR_ITEM_CORPSEOWNER))
	{
		removeAttribute(ATTR_ITEM_CORPSEOWNER);
	}

	if (newDuration > 0 && (!prevIt.stopTime || !hasAttribute(ATTR_ITEM_DURATION)))
	{
		setDecaying(DECAYING_FALSE);
		setDuration(newDuration);
	}
}

bool Item::hasSubType() const
{
	const ItemType& it = items[id];
	return it.hasSubType();
}

uint16_t Item::getSubType() const
{
	const ItemType& it = items[getID()];

	if (it.isFluidContainer() || it.isSplash())
	{
		return getFluidType();
	}
	else if (it.stackable)
	{
		return getItemCount();
	}
	else if (it.charges != 0)
	{
		return getCharges();
	}

	return getItemCount();
}

Player* Item::getHoldingPlayer()
{
	Cylinder* p = getParent();

	while (p)
	{
		if (p->getCreature())
			// Must be a player, creatures are not cylinders
		{
			return p->getCreature()->getPlayer();
		}

		p = p->getParent();
	}

	return NULL;
}

const Player* Item::getHoldingPlayer() const
{
	return const_cast<Item*>(this)->getHoldingPlayer();
}

const WeaponType_t& Item::getWeaponType() const
{
	return items[id].weaponType;
}

const Ammo_t& Item::getAmuType() const
{
	return items[id].amuType;
}

const uint32_t& Item::getShootRange() const
{
	return items[id].shootRange;
}

void Item::setSubType(const uint16_t& n)
{
	const ItemType& it = items[id];

	if (it.isFluidContainer() || it.isSplash())
	{
		setFluidType(n);
	}
	else if (it.stackable)
	{
		setItemCount(n);
	}
	else if (it.charges != 0)
	{
		setCharges(n);
	}
	else
	{
		setItemCount(n);
	}
}

Attr_ReadValue Item::readAttr(const AttrTypes_t& attr, PropStream& propStream)
{
	switch (attr)
	{
		case ATTR_COUNT:
		{
			uint8_t _count = 0;

			if (!propStream.GET_UINT8(_count))
			{
				return ATTR_READ_ERROR;
			}

			setSubType(_count);
			break;
		}
		case ATTR_ACTION_ID:
		{
			uint16_t _actionid = 0;

			if (!propStream.GET_UINT16(_actionid))
			{
				return ATTR_READ_ERROR;
			}

			setActionId(_actionid);
			break;
		}
		case ATTR_UNIQUE_ID:
		{
			uint16_t _uniqueid;

			if (!propStream.GET_UINT16(_uniqueid))
			{
				return ATTR_READ_ERROR;
			}

			setUniqueId(_uniqueid);
			break;
		}
		case ATTR_TEXT:
		{
			std::string _text;

			if (!propStream.GET_STRING(_text))
			{
				return ATTR_READ_ERROR;
			}

			setText(_text);
			break;
		}
		case ATTR_WRITTENDATE:
		{
			uint32_t _writtenDate;

			if (!propStream.GET_UINT32(_writtenDate))
			{
				return ATTR_READ_ERROR;
			}

			setWrittenDate(_writtenDate);
			break;
		}
		case ATTR_WRITTENBY:
		{
			std::string _writer;

			if (!propStream.GET_STRING(_writer))
			{
				return ATTR_READ_ERROR;
			}

			setWriter(_writer);
			break;
		}
		case ATTR_DESC:
		{
			std::string _text;

			if (!propStream.GET_STRING(_text))
			{
				return ATTR_READ_ERROR;
			}

			setSpecialDescription(_text);
			break;
		}
		case ATTR_RUNE_CHARGES:
		{
			uint8_t _charges = 1;

			if (!propStream.GET_UINT8(_charges))
			{
				return ATTR_READ_ERROR;
			}

			setSubType(_charges);
			break;
		}
		case ATTR_CHARGES:
		{
			uint16_t _charges = 1;

			if (!propStream.GET_UINT16(_charges))
			{
				return ATTR_READ_ERROR;
			}

			setSubType(_charges);
			break;
		}
		case ATTR_DURATION:
		{
			uint32_t duration = 0;

			if (!propStream.GET_UINT32(duration))
			{
				return ATTR_READ_ERROR;
			}

			if (((int32_t)duration) < 0)
			{
				duration = 0;
			}

			setDuration(duration);
			break;
		}
		case ATTR_DECAYING_STATE:
		{
			uint8_t state = 0;

			if (!propStream.GET_UINT8(state))
			{
				return ATTR_READ_ERROR;
			}

			if (state != DECAYING_FALSE)
			{
				setDecaying(DECAYING_PENDING);
			}

			break;
		}
		//these should be handled through derived classes
		//If these are called then something has changed in the items.xml since the map was saved
		//just read the values
		//Depot class
		case ATTR_DEPOT_ID:
		{
			uint16_t _depotId;

			if (!propStream.GET_UINT16(_depotId))
			{
				return ATTR_READ_ERROR;
			}

			return ATTR_READ_CONTINUE;
		}
		//Door class
		case ATTR_HOUSEDOORID:
		{
			uint8_t _doorId;

			if (!propStream.GET_UINT8(_doorId))
			{
				return ATTR_READ_ERROR;
			}

			return ATTR_READ_CONTINUE;
		}
		//Bed class
		case ATTR_SLEEPERGUID:
		{
			uint32_t _guid;

			if (!propStream.GET_UINT32(_guid))
			{
				return ATTR_READ_ERROR;
			}

			return ATTR_READ_CONTINUE;
		}
		case ATTR_SLEEPSTART:
		{
			uint32_t sleep_start;

			if (!propStream.GET_UINT32(sleep_start))
			{
				return ATTR_READ_ERROR;
			}

			return ATTR_READ_CONTINUE;
		}
		//Teleport class
		case ATTR_TELE_DEST:
		{
			TeleportDest tele_dest;

			if (!propStream.GET_UINT16(tele_dest._x) ||
			        !propStream.GET_UINT16(tele_dest._y) ||
			        !propStream.GET_UINT8(tele_dest._z))
			{
				return ATTR_READ_ERROR;
			}

			return ATTR_READ_CONTINUE;
		}
		//Container class
		case ATTR_CONTAINER_ITEMS:
		{
			uint32_t count;

			if (!propStream.GET_UINT32(count))
			{
				return ATTR_READ_ERROR;
			}

			//We cant continue parse attributes since there is
			//container data after this attribute.
			return ATTR_READ_ERROR;
		}
		default:
			return ATTR_READ_ERROR;
			break;
	}

	return ATTR_READ_CONTINUE;
}

bool Item::unserializeAttr(PropStream& propStream)
{
	uint8_t attr_type;

	while (propStream.GET_UINT8(attr_type) && attr_type != 0)
	{
		Attr_ReadValue ret = readAttr((AttrTypes_t)attr_type, propStream);

		if (ret == ATTR_READ_ERROR)
		{
			return false;
			break;
		}
		else if (ret == ATTR_READ_END)
		{
			return true;
		}
	}

	return true;
}

bool Item::unserializeItemNode(FileLoader& f, NODE node, PropStream& propStream)
{
	return unserializeAttr(propStream);
}

bool Item::serializeAttr(PropWriteStream& propWriteStream) const
{
	if (isStackable() || isFluidContainer() || isSplash())
	{
		uint8_t _count = getSubType();
		propWriteStream.ADD_UINT8(ATTR_COUNT);
		propWriteStream.ADD_UINT8(_count);
	}

	if (hasCharges())
	{
		uint16_t _count = getCharges();
		propWriteStream.ADD_UINT8(ATTR_CHARGES);
		propWriteStream.ADD_UINT16(_count);
	}

	if (!isNotMoveable() /*moveable*/)
	{
		uint16_t _actionId = getActionId();

		if (_actionId)
		{
			propWriteStream.ADD_UINT8(ATTR_ACTION_ID);
			propWriteStream.ADD_UINT16(_actionId);
		}
	}

	const std::string& _text = getText();

	if (_text.length() > 0)
	{
		propWriteStream.ADD_UINT8(ATTR_TEXT);
		propWriteStream.ADD_STRING(_text);
	}

	const time_t _writtenDate = getWrittenDate();

	if (_writtenDate > 0)
	{
		propWriteStream.ADD_UINT8(ATTR_WRITTENDATE);
		propWriteStream.ADD_UINT32(_writtenDate);
	}

	const std::string& _writer = getWriter();

	if (_writer.length() > 0)
	{
		propWriteStream.ADD_UINT8(ATTR_WRITTENBY);
		propWriteStream.ADD_STRING(_writer);
	}

	const std::string& _specialDesc = getSpecialDescription();

	if (_specialDesc.length() > 0)
	{
		propWriteStream.ADD_UINT8(ATTR_DESC);
		propWriteStream.ADD_STRING(_specialDesc);
	}

	if (hasAttribute(ATTR_ITEM_DURATION))
	{
		uint32_t duration = getDuration();
		propWriteStream.ADD_UINT8(ATTR_DURATION);
		propWriteStream.ADD_UINT32(duration);
	}

	uint32_t decayState = getDecaying();

	if (decayState == DECAYING_TRUE || decayState == DECAYING_PENDING)
	{
		propWriteStream.ADD_UINT8(ATTR_DECAYING_STATE);
		propWriteStream.ADD_UINT8(decayState);
	}

	return true;
}

bool Item::isPushable() const
{
	return !isNotMoveable();
}

int Item::getThrowRange() const
{
	return (isPickupable() ? 15 : 2);
}

bool Item::hasProperty(const ITEMPROPERTY& prop) const
{
	const ItemType& it = items[id];

	switch (prop)
	{
		case BLOCKSOLID:

			if (it.blockSolid)
			{
				return true;
			}

			break;
		case MOVEABLE:

			if (it.moveable && getUniqueId() == 0)
			{
				return true;
			}

			break;
		case HASHEIGHT:

			if (it.hasHeight)
			{
				return true;
			}

			break;
		case BLOCKPROJECTILE:

			if (it.blockProjectile)
			{
				return true;
			}

			break;
		case BLOCKPATH:

			if (it.blockPathFind)
			{
				return true;
			}

			break;
		case ISVERTICAL:

			if (it.isVertical)
			{
				return true;
			}

			break;
		case ISHORIZONTAL:

			if (it.isHorizontal)
			{
				return true;
			}

			break;
		case IMMOVABLEBLOCKSOLID:

			if (it.blockSolid && (!it.moveable || getUniqueId() != 0))
			{
				return true;
			}

			break;
		case IMMOVABLEBLOCKPATH:

			if (it.blockPathFind && (!it.moveable || getUniqueId() != 0))
			{
				return true;
			}

			break;
		case SUPPORTHANGABLE:

			if (it.isHorizontal || it.isVertical)
			{
				return true;
			}

			break;
		case IMMOVABLENOFIELDBLOCKPATH:

			if (!it.isMagicField() && it.blockPathFind && (!it.moveable || getUniqueId() != 0))
			{
				return true;
			}

			break;
		case NOFIELDBLOCKPATH:

			if (!it.isMagicField() && it.blockPathFind)
			{
				return true;
			}

			break;
		default:
			return false;
	}

	return false;
}

bool Item::isBlocking(const Creature* creature) const
{
	return items[id].blockSolid;
}

bool Item::isStackable() const
{
	return items[id].stackable;
}

bool Item::isRune() const
{
	return items[id].isRune();
}

bool Item::isFluidContainer() const
{
	return (items[id].isFluidContainer());
}

bool Item::isAlwaysOnTop() const
{
	return items[id].alwaysOnTop;
}

bool Item::isGroundTile() const
{
	return items[id].isGroundTile();
}

bool Item::isSplash() const
{
	return items[id].isSplash();
}

bool Item::isMagicField() const
{
	return items[id].isMagicField();
}

bool Item::isNotMoveable() const
{
	return !items[id].moveable;
}

bool Item::isMoveable() const
{
	return items[id].moveable;
}

bool Item::isPickupable() const
{
	return items[id].pickupable;
}

bool Item::isWeapon() const
{
	return (items[id].weaponType != WEAPON_NONE);
}

bool Item::isUseable() const
{
	return items[id].useable;
}

bool Item::isHangable() const
{
	return items[id].isHangable;
}

bool Item::isRoteable() const
{
	const ItemType& it = items[id];
	return it.rotable && it.rotateTo;
}

bool Item::isDoor() const
{
	return items[id].isDoor();
}

bool Item::isBed() const
{
	return items[id].isBed();
}

bool Item::isLevelDoor() const
{
	return items[id].isLevelDoor();
}

bool Item::hasCharges() const
{
	return getCharges() > 0;
}

bool Item::isSolidForItems() const
{
	return items[id].isSolidForItems();
}

bool Item::floorChangeDown() const
{
	return items[id].floorChangeDown;
}

bool Item::floorChangeNorth() const
{
	return items[id].floorChangeNorth;
}

bool Item::floorChangeSouth() const
{
	return items[id].floorChangeSouth;
}

bool Item::floorChangeEast() const
{
	return items[id].floorChangeEast;
}

bool Item::floorChangeWest() const
{
	return items[id].floorChangeWest;
}

const std::string& Item::getName() const
{
	return items[id].name;
}

const std::string& Item::getPluralName() const
{
	return items[id].pluralName;
}

double Item::getWeight() const
{
	if (isStackable())
	{
		return items[id].weight * std::max((int32_t)1, (int32_t)getItemCount());
	}

	return items[id].weight;
}

const int& Item::getAttack() const
{
	return items[id].attack;
}

const int& Item::getArmor() const
{
	return items[id].armor;
}

const int& Item::getDefense() const
{
	return items[id].defense;
}

const int& Item::getExtraDef() const
{
	return items[id].extraDef;
}

const uint32_t& Item::getSlotPosition() const
{
	return items[id].slot_position;
}

const uint16_t& Item::getWieldPosition() const
{
	return items[id].wield_position;
}

const int& Item::getHitChance() const
{
	return items[id].hitChance;
}

bool Item::isReadable() const
{
	return items[id].canReadText;
}

bool Item::canWriteText() const
{
	return items[id].canWriteText;
}

const uint16_t& Item::getMaxWriteLength() const
{
	return items[id].maxTextLen;
}

std::string Item::getLongName(const ItemType& it, const int32_t& lookDistance,
                              const Item* item /*= NULL*/, int32_t subType /*= -1*/, bool addArticle /*= true*/)
{
	std::ostringstream s;

	if (item)
	{
		subType = item->getSubType();
	}

	if (it.name.length())
	{
		if (it.stackable && subType > 1)
		{
			if (it.showCount)
			{
				s << subType << " ";
			}

			s << it.pluralName;
		}
		else
		{
			if (addArticle && !it.article.empty())
			{
				s << it.article << " ";
			}

			s << it.name;
		}
	}
	else
	{
		s << "an item of type " << it.id << ", please report it to gamemaster";
	}

	return s.str();
}

std::string Item::getLongName() const
{
	const ItemType& it = items[id];
	return getLongName(it, 0, this);
}

const uint8_t& Item::getItemCount() const
{
	return count;
}

void Item::setItemCount(const uint8_t& n)
{
	count = n > 1 ? n : 0;
}

uint32_t Item::countByType(const Item* i, const int32_t& subType)
{
	if (subType == -1 || subType == i->getSubType())
	{
		return i->getItemCount();
	}

	return 0;
}

std::string Item::getDescription(const ItemType& it, const int32_t& lookDistance,
                                 const Item* item /*= NULL*/, int32_t subType /*= -1*/, bool addArticle /*= true*/)
{
	std::stringstream s;

	if (item)
	{
		subType = item->getSubType();
	}

	s << getLongName(it, lookDistance, item, subType, addArticle);

	if (it.isRune())
	{
		s << " (\"" << it.runeSpellName << "\").";

		if (it.runeLevel > 0 || it.runeMagLevel > 0)
		{
			s << std::endl << "It can only be used with";

			if (it.runeLevel > 0)
			{
				s << " level " << it.runeLevel;
			}

			if (it.runeMagLevel > 0)
			{
				if (it.runeLevel > 0)
				{
					s << " and";
				}

				s << " magic level " << it.runeMagLevel;
			}

			s << " or higher.";
		}
	}
	else if (it.weaponType != WEAPON_NONE)
	{
		if (it.weaponType == WEAPON_DIST && it.amuType != AMMO_NONE)
		{
			s << " (Range:" << it.shootRange;

			if (it.attack != 0)
			{
				s << ", Atk" << std::showpos << it.attack << std::noshowpos;
			}

			if (it.hitChance > 0) //excludes both cases it.hitchance 0 and -1
			{
				s << ", Hit%" << std::showpos << it.hitChance << std::noshowpos;
			}

			s << ")";
		}
		else if (it.weaponType != WEAPON_AMMO && it.weaponType != WEAPON_WAND) // Arrows and Bolts doesn't show atk
		{
			s << " (";

			if (it.attack != 0)
			{
				s << "Atk:" << (int32_t)it.attack;

				if (it.abilities.elementDamage != 0)
				{
					s << " " << "physical + ";

					switch (it.abilities.elementType)
					{
						case COMBAT_ICEDAMAGE:
							s << it.abilities.elementDamage << " Ice,";
							break;
						case COMBAT_EARTHDAMAGE:
							s << it.abilities.elementDamage << " Earth,";
							break;
						case COMBAT_FIREDAMAGE:
							s << it.abilities.elementDamage << " Fire,";
							break;
						case COMBAT_ENERGYDAMAGE:
							s << it.abilities.elementDamage << " Energy,";
							break;
						default:
							s << it.abilities.elementDamage << " Unknown,";
							break;
					}
				}
			}

			if (it.defense != 0 || it.extraDef != 0)
			{
				if (it.attack != 0)
				{
					s << " ";
				}

				s << "Def:" << (int32_t)it.defense;

				if (it.extraDef != 0)
				{
					s << " " << std::showpos << (int32_t)it.extraDef << std::noshowpos;
				}
			}

			if (it.abilities.stats[STAT_MAGICPOINTS] != 0)
			{
				if (it.attack != 0 || it.defense != 0 || it.extraDef != 0)
				{
					s << ", ";
				}

				s << "magic level " << std::showpos << (int32_t)it.abilities.stats[STAT_MAGICPOINTS] << std::noshowpos;
			}

			s << ")";
		}

		if (it.showCharges)
		{
			if (subType > 1)
			{
				s << " that has " << (int32_t)subType << " charges left";
			}
			else
			{
				s << " that has 1 charge left";
			}
		}

		s << ".";
	}
	else if (it.armor != 0 || it.abilities.skill.any() || it.abilities.absorb.any() || it.abilities.stats[STAT_MAGICPOINTS] != 0 || it.abilities.speed != 0 || it.defense != 0)
	{
		if (it.showCharges)
		{
			if (subType > 1)
			{
				s << " that has " << (int32_t)subType << " charges left";
			}
			else
			{
				s << " that has 1 charge left";
			}
		}
		else if (it.showDuration)
		{
			if (item && item->hasAttribute(ATTR_ITEM_DURATION))
			{
				int32_t duration = item->getDuration() / 1000;
				s << " that has energy for ";

				if (duration >= 120)
				{
					s << duration / 60 << " minutes left.";
				}
				else if (duration > 60)
				{
					s << "1 minute left.";
				}
				else
				{
					s << "less than a minute left.";
				}
			}
			else
			{
				s << " that is brand-new.";
			}
		}

		s << " (";
		bool prevDesc = false;

		if (it.armor != 0)
		{
			// as it is the first desc it isn't
			// really necessary to add it here
			//if(prevDesc)
			//	s << ", ";
			s << "Arm:" << it.armor;
			prevDesc = true;
		}

		if (it.abilities.skill.any())
		{
			if (prevDesc)
			{
				s << ", ";
			}

			it.abilities.skill.getDescription(s);
			prevDesc = true;
		}

		if (it.abilities.absorb.any())
		{
			if (prevDesc)
			{
				s << ", ";
			}

			s << "protection";
			it.abilities.absorb.getDescription(s);
			prevDesc = true;
		}

		if (it.abilities.stats[STAT_MAGICPOINTS] != 0)
		{
			if (prevDesc)
			{
				s << ", ";
			}

			s << "magic level " << std::showpos << (int32_t)it.abilities.stats[STAT_MAGICPOINTS] << std::noshowpos;
			prevDesc = true;
		}

		if (it.defense != 0)
		{
			if (prevDesc)
			{
				s << ", ";
			}

			s << "defense " << std::showpos << (int32_t)it.defense << std::noshowpos;
			prevDesc = true;
		}

		if (it.abilities.speed != 0)
		{
			if (prevDesc)
			{
				s << ", ";
			}

			s << "speed " << std::showpos << (int32_t)(it.abilities.speed / 2) << std::noshowpos;
			// last desc... same thing as the first
			//prevDesc = true;
		}

		s << ").";
	}
	else if (it.isFluidContainer())
	{
		if (subType > 0)
		{
			s << " of " << items[subType].name << ".";
		}
		else
		{
			s << ". It is empty.";
		}
	}
	else if (it.isSplash())
	{
		s << " of ";

		if (subType > 0)
		{
			s << items[subType].name << ".";
		}
		else
		{
			s << items[1].name << ".";
		}
	}
	else if (it.isContainer())
	{
		s << " (Vol:" << (int32_t)it.maxItems << ").";
	}
	else if (it.isKey())
	{
		if (item)
		{
			s << " (Key:" << (int32_t)item->getActionId() << ").";
		}
		else
		{
			s << " (Key:0).";
		}
	}
	else if (it.allowDistRead)
	{
		s << std::endl;

		if (item && item->getText() != "")
		{
			if (lookDistance <= 4)
			{
				if (item->getWriter().length())
				{
					s << item->getWriter() << " wrote";
					time_t wDate = item->getWrittenDate();

					if (wDate > 0)
					{
						char date[16];
						formatDateShort(wDate, date);
						s << " on " << date;
					}

					s << ": ";
				}
				else
				{
					s << "You read: ";
				}

				s << item->getText();
			}
			else
			{
				s << "You are too far away to read it.";
			}
		}
		else
		{
			s << "Nothing is written on it.";
		}
	}
	else if (it.isLevelDoor() && item && item->getActionId() >= 1000)
	{
		s << " for level " << item->getActionId() - 1000;
	}
	else if (it.showCharges)
	{
		if (subType > 1)
		{
			s << " that has " << (int32_t)subType << " charges left.";
		}
		else
		{
			s << " that has 1 charge left.";
		}
	}
	else if (it.showDuration)
	{
		if (item && item->hasAttribute(ATTR_ITEM_DURATION))
		{
			int32_t duration = item->getDuration() / 1000;
			s << " that has energy for ";

			if (duration >= 120)
			{
				s << duration / 60 << " minutes left.";
			}
			else if (duration > 60)
			{
				s << "1 minute left.";
			}
			else
			{
				s << "less than a minute left.";
			}
		}
		else
		{
			s << " that is brand-new.";
		}
	}
	else
	{
		s << ".";
	}

	if (it.wieldInfo != 0)
	{
		s << std::endl << "It can only be wielded properly by ";

		if (it.wieldInfo & WIELDINFO_PREMIUM)
		{
			s << "premium ";
		}

		if (it.wieldInfo & WIELDINFO_VOCREQ)
		{
			s << it.vocationString;
		}
		else
		{
			s << "players";
		}

		if (it.wieldInfo & WIELDINFO_LEVEL)
		{
			s << " of level " << (int32_t)it.minReqLevel << " or higher";
		}

		if (it.wieldInfo & WIELDINFO_MAGLV)
		{
			if (it.wieldInfo & WIELDINFO_LEVEL)
			{
				s << " and";
			}
			else
			{
				s << " of";
			}

			s << " magic level " << (int32_t)it.minReqMagicLevel << " or higher";
		}

		s << ".";
	}

	if (lookDistance <= 1)
	{
		double weight = (!item ? it.weight : item->getWeight());

		if (weight > 0)
		{
			s << std::endl << getWeightDescription(it, weight);
		}
	}

	if (item && item->getSpecialDescription() != "")
	{
		s << std::endl << item->getSpecialDescription().c_str();
	}
	else if (it.description.length() && lookDistance <= 1)
	{
		s << std::endl << it.description;
	}
	else if(it.isFluidContainer() && subType > 0 && items[subType].description.length() && lookDistance <= 1)
	{
		s << std::endl << items[subType].description;
	}

	return s.str();
}

std::string Item::getDescription(const int32_t& lookDistance) const
{
	std::stringstream s;
	const ItemType& it = items[id];
	s << getDescription(it, lookDistance, this);
	return s.str();
}

std::string Item::getXRayDescription() const
{
	std::stringstream ret;
	ret << "ID: " << getID() << std::endl;
	uint16_t actionId = getActionId();
	uint16_t uniqueId = getUniqueId();

	if (actionId > 0)
	{
		ret << "Action ID: " << actionId << std::endl;
	}

	if (uniqueId > 0)
	{
		ret << "Unique ID: " << uniqueId << std::endl;
	}

#ifdef __DEBUG__

	if (getContainer())
	{
		ret << "There are " << getContainer()->getTotalAmountOfItemsInside() - 1 << " things inside of this." << std::endl;
	}

#endif
	ret << Thing::getXRayDescription();
	return ret.str();
}

std::string Item::getWeightDescription() const
{
	double weight = getWeight();

	if (weight > 0)
	{
		return getWeightDescription(weight);
	}
	else
	{
		return "";
	}
}

const uint16_t& Item::getID() const
{
	return id;
}

const uint16_t& Item::getClientID() const
{
	return items[id].clientId;
}

std::string Item::getWeightDescription(const double& weight) const
{
	const ItemType& it = Item::items[id];
	return getWeightDescription(it, weight, getItemCount());
}

std::string Item::getWeightDescription(const ItemType& it, double weight, const uint32_t& count /*= 1*/)
{
	std::stringstream ss;

	if (it.stackable && count > 1)
	{
		ss << "They weigh " << std::fixed << std::setprecision(2) << weight << " oz.";
	}
	else
	{
		ss << "It weighs " << std::fixed << std::setprecision(2) << weight << " oz.";
	}

	return ss.str();
}


void Item::setUniqueId(const uint16_t& n)
{
	if (getUniqueId() != 0)
	{
		return;
	}

	ItemAttributes::setUniqueId(n);
	ScriptEnviroment::addUniqueThing(this);
}

void Item::setDefaultDuration()
{
	uint32_t duration = getDefaultDuration();

	if (duration != 0)
	{
		setDuration(duration);
	}
}

uint32_t Item::getDefaultDuration() const
{
	return items[id].decayTime * 1000;
}

bool Item::canDecay()
{
	if (isRemoved())
	{
		return false;
	}

	if (getUniqueId() != 0)
	{
		return false;
	}

	const ItemType& it = Item::items[id];

	if (it.decayTo == -1 || it.decayTime == 0)
	{
		return false;
	}

	return true;
}

bool Item::canRemove() const
{
	return true;
}

bool Item::canTransform() const
{
	return true;
}

bool Item::onTradeEvent(const TradeEvents_t& event, Player* owner)
{
	return true;
}

uint32_t Item::getWorth() const
{
	const ItemType& it = Item::items[id];
	return getItemCount() * it.currency;
}

void Item::getLight(LightInfo& lightInfo)
{
	const ItemType& it = items[id];
	lightInfo.color = it.lightColor;
	lightInfo.level = it.lightLevel;
}

void Item::__startDecaying()
{
	g_game.startDecay(this);
}

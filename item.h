//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Item represents an existing item.
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


#ifndef __OTSERV_ITEM_H
#define __OTSERV_ITEM_H

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <iostream>
#include <list>
#include <vector>

#include "texcept.h"

#include "thing.h"
#include "items.h"



class Creature;


class Item : public Thing
{
    private:
        unsigned id;  // the same id as in ItemType
	      unsigned char count; // number of stacked items

    private: // the following, I will have to rethink:
        // could be union:

			unsigned short maxitems; //number of max items in container  
			unsigned short actualitems; // number of items in container
        // list of items if this is a container
        std::list<Item *> lcontained;

    public:

		static Items items;

   unsigned short getID() const;    // ID as in ItemType
   void setID(unsigned short newid);
		
    bool isWeapon() const;
	 WeaponType getWeaponType() const;


    bool isBlocking() const;
		bool isStackable() const;
    bool isMultiType() const;
		bool isAlwaysOnTop() const;
		bool isGroundTile() const;
		bool isNotMoveable() const;
		bool isContainer() const;

		int use(){std::cout << "use " << id << std::endl; return 0;};
		int use(Item*){std::cout << "use with item ptr " << id << std::endl; return 0;};
		int use(Creature*){std::cout << "use with creature ptr " << id << std::endl; return 0;};
		std::string getDescription();
		std::string getName();
		int unserialize(xmlNodePtr p);
		xmlNodePtr serialize();

        // get the number of items
        unsigned char getItemCountOrSubtype() const;

        // Constructor for items
        Item(const unsigned short _type);
        Item(const unsigned short _type, unsigned char _count);
		Item();

        ~Item();

        // definition for iterator over backpack itemsfclose(f);
				int getContainerItemCount() {return actualitems;};
				int getContainerMaxItemCount() {return maxitems;};
        typedef std::list<Item *>::const_iterator iterator;
        iterator getItems();     // begin();
        iterator getEnd();       // iterator beyond the last element
        void addItem(Item* newitem);     // add an item to the container
				void removeItem(Item* item); //remove an item from the container
				void moveItem(unsigned char from_slot, unsigned char to_slot);
				Item* getItem(unsigned long slot_num);
				void isContainerHolding(Item* item, bool& found); //search all containers for the item recursively
        Item& operator<<(Item*); // put items into the container
};


#endif

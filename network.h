//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// methods to access the network...
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
// $Id$
//////////////////////////////////////////////////////////////////////
// $Log$
// Revision 1.7  2004/11/19 21:39:26  shivoc
// fix a bug in converting from ascii to binary representation
//
// Revision 1.6  2002/05/28 13:55:56  shivoc
// some minor changes
//
//
//////////////////////////////////////////////////////////////////////

#ifndef __network_h
#define __network_h

// standard includes...
#include "definitions.h"
#include "texcept.h"

// STL-headers
#include <string>

namespace TNetwork {
    // Functions to control the server...
    Socket make_socket(int, short unsigned int) throw(texception);
    void ShutdownServer(const Socket&) throw();

    // Functions to send messages to the client...

    // send Data to the client (length, data)
    void SendData(const Socket&, const std::string&) throw();

    // receive Data from the Client (length, data)
    // if there is data (like after accepting the socket) and the check for new
    // data should be omitted, check should be set to false
    std::string ReceiveData(const Socket&) throw(texception);

    // shutdown a client connection...
    void CloseSocket(const Socket&) throw();

    // accept a connection from a player and return the socket...
    Socket AcceptPlayer(const Socket&) throw(texception);

	 // convert ip address in char* to uint32_t
	 uint32_t convip(const char* ip);

} // Namespace TNetwork 

#endif // __network_h

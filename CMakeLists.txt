CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(otserv)

# setup custom cmake modules path
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

# choose database type
SET(DATABASE_TYPE sqlite CACHE "Database type" STRING)
MESSAGE(STATUS "Database type: " ${DATABASE_TYPE})

# choose a default build type if not specified
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
ENDIF(NOT CMAKE_BUILD_TYPE)
MESSAGE(STATUS "BUILD TYPE: " ${CMAKE_BUILD_TYPE})

# find needed packages
FIND_PACKAGE(Boost COMPONENTS system thread regex REQUIRED)
FIND_PACKAGE(Lua REQUIRED)
FIND_PACKAGE(GMP REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)

IF(DATABASE_TYPE STREQUAL "mysql")
    FIND_PACKAGE(Mysql REQUIRED)
ENDIF(DATABASE_TYPE STREQUAL "mysql")

IF(DATABASE_TYPE STREQUAL "sqlite")
    FIND_PACKAGE(Sqlite REQUIRED)
ENDIF(DATABASE_TYPE STREQUAL "sqlite")

FIND_PACKAGE(ZLIB REQUIRED)

# setup compiler options
IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CXX_WARNS "")
    SET(CMAKE_CXX_FLAGS "-std=gnu++0x -pipe ${CXX_WARNS}")
    SET(CMAKE_CXX_FLAGS_DEBUG " -O0 -g3 -ggdb3 -fno-inline")
    SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O1 -g -ggdb -fno-inline")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)


INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR}
    ${GMP_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
    ${SQLITE_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
)

ADD_DEFINITIONS(-D__USE_SQLITE__)

SET(SOURCES
    src/admin.cpp
    src/account.cpp
    src/actions.cpp
    src/allocator.cpp
    src/ban.cpp
    src/baseevents.cpp
    src/beds.cpp
    src/chat.cpp
    src/combat.cpp
    src/condition.cpp
    src/configmanager.cpp
    src/connection.cpp
    src/container.cpp
    src/creature.cpp
    src/creatureevent.cpp
    src/cylinder.cpp

    # database
    src/database.cpp
    src/databasesqlite.cpp
    src/databasemysql.cpp
    #src/databaseodbc.cpp
    #src/databasepqsql.cpp

    src/depot.cpp
    src/exception.cpp
    src/fileloader.cpp
    src/game.cpp
    src/globalevent.cpp
    src/guild.cpp
    src/house.cpp
    src/housetile.cpp
    src/ioaccount.cpp
    src/iomapotbm.cpp
    src/iomapserialize.cpp
    src/ioplayer.cpp
    src/item.cpp
    src/items.cpp
    src/logger.cpp
    src/luascript.cpp
    src/mailbox.cpp
    src/map.cpp
    src/md5.cpp
    src/monster.cpp
    src/monsters.cpp
    src/movement.cpp
    src/networkmessage.cpp
    src/npc.cpp
    src/otpch.cpp
    src/otserv.cpp
    src/outfit.cpp
    src/outputmessage.cpp
    src/party.cpp
    src/player.cpp
    src/position.cpp
    src/protocol.cpp
    src/protocolold.cpp
    src/protocolgame.cpp
    src/protocollogin.cpp
    src/quests.cpp
    src/raids.cpp
    src/rsa.cpp
    src/scheduler.cpp
    src/scriptmanager.cpp
    src/server.cpp
    src/spawn.cpp
    src/sha1.cpp
    src/spells.cpp
    src/status.cpp
    src/talkaction.cpp
    src/tasks.cpp
    src/teleport.cpp
    src/thing.cpp
    src/tile.cpp
    src/tools.cpp
    src/town.cpp
    src/trashholder.cpp
    src/vocation.cpp
    src/waitlist.cpp
    src/weapons.cpp
)

# target executable
ADD_EXECUTABLE(otserv ${SOURCES})

IF(WIN32)
    SET(ADDITIONAL_LIBRARIES ws2_32 mswsock)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthreads")
ELSE(WIN32)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF(WIN32)

# target link libraries
TARGET_LINK_LIBRARIES(
    otserv
    ${Boost_LIBRARIES}
    ${LUA_LIBRARIES}
    ${GMP_LIBRARY}
    ${LIBXML2_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${MYSQL_LIBRARY}
    ${SQLITE_LIBRARIES}
    ${ADDITIONAL_LIBRARIES}
)


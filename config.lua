-------- config.lua --------
-- Config file for OTServ --
----------------------------

-- data directory location
datadir = "data/"

-- map location
map = "data/world/map.otbm"

-- Type of map storage,
-- 'relational' - Slower, but possible to run database queries to change all items to another id for example.
-- 'binary' - Faster, but you cannot run DB queries.
-- To switch, load server with the current type, change the type in config.lua
-- type /reload config and the save the server with /closeserver serversave
map_store_type = "binary"

-- mapkind
-- options: OTBM for binary map, XML for OTX map
mapkind = "OTBM"

-- server name
servername = "OTServ"

-- server location
location = "Earth"

-- server ip (the ip that server listens on)
ip = "127.0.0.1"

-- login server port
-- This should be the port used for connecting with IP changers etc.
login_port = "7171"

-- game server port
-- game server must be on it's own port (due to limits of the protocol)
game_port = "7172"

-- status port
-- Used by status protocol connections, should be same as login
-- to work correctly with server lists etc.
status_port = "7171"

-- admin port
-- Port used by the OTAdmin protocol
admin_port = "7171"

-- server url
url = "http://otfans.net"

-- server owner name
ownername = "Derp"

-- server owner email
owneremail = "derp@none"

-- world name
worldname = "OpenTibia"

-- world type
-- options: openpvp/pvp, optionalpvp/no-pvp, hardcorepvp/pvp-enforced
worldtype = "openpvp"

-- does player rate (from lua functions) count on experience gained from killings players on pvp-e worlds?
rates_for_player_killing = false

-- exhausted time in ms (1000 = 1 second) for "yell" speak type
exhausted = 30000

-- exhausted time in ms (1000 = 1 second) for aggressive spells/weapons
fightexhausted = 2000

-- exhausted time in ms (1000 = 1 second) for none-aggressive spells
healexhausted = 1000

-- exhausted time in ms (1000 = 1 second) for floor changing
stairhop_exhausted = 2000

-- how many ms to add if the player is already exhausted and tries to cast a spell/yells (1000 = 1 second)
exhaustedadd = 5000

-- how long does the player has to stay out of fight to get pz unlocked in ms (1000 = 1 second)
in_fight_duration = 60000

-- how long a field belongs to a player before it no longer causes PZ lock for the owner
field_ownership_duration = 5000

-- When a player dies, how many of the assisting players (ie. how many of those who made damage) should be stored in database?
death_assist_count = 19

-- Should only the player who did the last hit get the 15-minute pz lock?
last_hit_pzblock_only = true

-- enable (advanced, item based) hotkeys?
enablehotkeys = true

-- minimum amount of time between actions ('Use') (1000 = 1 second)
minactioninterval = 200

-- minimum amount of time between extended actions ('Use with...') (1000 = 1 second)
minactionexinterval = 1000

-- house rent period
-- options: daily, weekly, monthly
houserentperiod = "monthly"

-- whether or not to check for premium accounts when using beds
premonlybeds = true

-- Should the server use account balance system or depot system for paying houses?
use_balance_house_paying = true

-- How long time the red skull will last in seconds (default 30 days)
red_skull_duration = 30*24*60*60

-- How long time the black skull will last in seconds (default 45 days)
black_skull_duration = 45*24*60*60

-- White skull duration, how long a player will get pz locked/white skull
unjust_skull_duration = 15*60*1000

-- Enable this and players will get pzlocked regardless of being the aggressor or not (<8.5 this was set to true)
defensive_pz_lock = false

-- How many players you need to kill to gain a red skull per day/week/month
-- Set 0 to disable
kills_per_day_red_skull = 3
kills_per_week_red_skull = 5
kills_per_month_red_skull = 10

-- How many players you need to kill to gain a black skull per day/week/month
-- Set 0 to disable
kills_per_day_black_skull = 6
kills_per_week_black_skull = 10
kills_per_month_black_skull = 20

-- Broadcast violation window banishments
broadcast_banishments = "no"

-- Violation window config
max_violation_comment_size = 255
notations_to_banishment = 3
warnings_to_final_banishment = 4
warnings_to_deletion = 5
banishment_length = 7*86400 --seconds
final_banishment_length = 30*86400 --seconds
ip_banishment_length = 86400 --seconds

-- Extra experience % per player when sharing experience
party_exp_mul = 5

-- Anti-MageBomb system
-- This system works as in Tibia, time in milliseconds
login_attack_delay = 10*1000

-- AFK Configuration (in milliseconds, but use values for minutes)
-- Maximum time a player can stay away from keyboard before being kicked
max_idle_time = 16*60*1000
-- Player will be warned about being kicked at this time
max_idle_time_warning = 15*60*1000

-- Should just premium players be able to use addons?
addons_only_for_premium = true

-- Exp Stages configuration is in /data/exp_stages.lua
experience_stages = false

-- rates (experience, skill, magic level, loot and spawn)
rate_exp = 1
rate_exp_pvp = 0
rate_skill = 1
rate_mag = 1
rate_loot = 1
rate_spawn = 1

--Displays the skill advanced to when advancing in skills (melee) [default: "no"]
--Example: "You advanced to sword fighting 17. "
show_new_skill_level = false

--Displays amount of health healed.
--Similar to the way damage is shown, except in green numbers (Example: +17) [default: "no"]
show_healing = false

--Use orange text for spells rather than traditional text [default: "no"]
--NOTE: The orange text will not be displayed on the default channel
orange_spell_text = false

-- Configuration for death messages
-- Should the client show the popup window after a death? [default: "yes"]
show_death_window = true
-- What message should be sent for the player, after its own death? Use "" to disable.
death_msg = "You are dead."

--should players be able to rope creatures (like at older versions of tibia)?
can_rope_creatures = true

--how many floors away of a player should a monster be to become idle?
--WARNING: setting this as any number bigger than 3 may increase CPU usage and
--setting this to any value smaller than 2 will make nearly impossible to rope monsters from a hole
height_minimum_for_idle = 3

-- Stamina Configuration
-- Gain stamina rate - 1 second offline = X milliseconds of stamina
rate_stamina_gain = 334
-- Slow gain stamina rate. This is used if you are gaining stamina that gives extra experience
-- 1 second offline = X milliseconds of stamina
slow_rate_stamina_gain = 100
-- Loss stamina rate - works as other rates
rate_stamina_loss = 1
-- For how many milliseconds (from 42 hours) you get extra experience from monsters
stamina_extra_experience_duration = 2 * 60 * 60 * 1000
-- How much extra XP you get 0.5 is 50% extra
stamina_extra_experience_rate = 0.5
-- If true, only premium players gain extra experience
stamina_extra_experience_onlyprem = true
-- For how many milliseconds after killing a monster stamina will keep being removed
hunting_kill_duration = pzlocked

-- Remove ammunition
-- If false, ammunition will not be removed when using distance weapons
-- (or other weapons that use ammunition)
remove_ammunition = true

-- Remove rune charges
-- This only applies to runes done using the default functions.
-- Custom runes made using actions will not be affected.
remove_rune_charges = true

-- Remove weapon charges
-- Set to false to disable charges disappearing from weapons on use
remove_weapon_charges = true

-- Distance weapon configuration
-- Should exhaustion and action time be checked for using a distance weapon?
distance_weapon_interrupt_swing = true

-- Wands configuration exhaustion
-- Should exhaustion and action time be checked for using wands?
wands_interrupt_swing = true

-- maximum amount of items inside of a container (including the items inside of any container inside of it)
-- Use 0 for infinite
max_amount_items_inside_containers = 5000

--maximal length for chains of containers (i.e, a container inside of a container which is inside of a container...)
max_deepness_of_chain_of_containers = 500

--should OTServ bind only global IP address ?
bind_only_global_address = false

-- How many items can be stacked in a single tile (all type of tiles)(client side)? DO NOT CHANGE IT UNLESS THAT YOU KNOW WHAT YOU ARE DOING
max_stack_size = 1000

-- How many items can be stacked in a normal tile ? (0 = max_stack_size)
tile_limit = 0

-- How many items can be stacked in a protection zone tile ? (0 = max_stack_size)
protection_tile_limit = 0

-- How many items can be stacked in a house zone tile ? (0 = max_stack_size)
house_tile_limit = 0


-- fist fighting configuration
-- How strong should be the attack using bare hands (fist fighting)?
fist_strength = 7

-- Guild War Fee
-- The addition 'war charge' to start a guild war (default: 1000)
-- To disable set to 0
guild_war_fee = 1000

-- Allow gamemaster multiclient login to everybody?
allow_gamemaster_multiclient = false

-- despawn configs
-- how many floors can a monster go from his spawn before despawning
despawnrange = 2
-- how many square metters can a monster be far from his spawn before despawning
despawnradius = 50

--should players be able to hit invisible creatures using not AOE runes? (default: false)
can_attack_invisible = false

--should players be able to pass through each other if they can't attack each other (non-pvp servers/tiles or levels too low for PvP)?
--this option also controls if they can pass through magic wall/wild growth casted by players with those restrictions
can_pass_through = true

--should magic walls (and wild growth) disappear when a player walks upon it? (non-pvp servers/tiles or casted by chars who are too low)?
magic_wall_disappear_on_walk = true

--players with a level smaller than this number can't be attacked by other players. Set it to to 0 to disable
min_pvp_level = 0

--how much % of the total damage players actually receive when attacked by other players?
pvp_damage = 50

--how much % of the total damage black skulled players actually receive when attacked by other players?
pvp_damage_at_black_skulls = 100

--should pvp restrictions apply to summons too?
-- (experimental code - requires otserv to be compiled with flag __MIN_PVP_LEVEL_APPLIES_TO_SUMMONS__)
min_pvp_level_applies_to_summons = true

-- How many milliseconds (1 sec = 1000) we should wait until the player will be pushed by another player.
push_interval = 2000

-- How many milliseconds a player should wait until he can move another item after moving one (0 disables it)
move_item_interval = 500

-- max number of messages a player can say before getting muted (default 4), set to 0 to disable muting
maxmessagebuffer = 4

-- max amount of none stackable items that a NPC will sell to a player at the same time
npc_max_nonestackable_sell_amount = 100

-- if you doesn't wish to see errors like this: "Error: Expected an integer/boolean parameter"
--(it means that you are missing an return somewhere but this is not so important and is just spamming your console on a production server) then set this to false
lua_excepted_type_errors_enabled = true

-- motd (the message box that you sometimes get before you choose characters)
motd = "Welcome to OTServ!"
motdnum = "1"

-- login message
loginmsg = "Welcome to OTServ. For help visit http://otfans.net."

-- how many logins attempts until ip is temporary disabled
-- set to 0 to disable
logintries = 5

-- how long the retry timeout until a new login can be made (without disabling the ip)
retrytimeout = 5000

-- how long the player need to wait until the ip is allowed again
logintimeout = 60 * 1000

-- how deep queryDestination should look for free slots (default -1, infinite)
player_querydestination_deepness = -1

-- allow clones (multiple logins of the same char)
allowclones = true

-- only one player online per account
checkaccounts = false

-- max number of players allowed
maxplayers = "900"

-- save client debug assertion reports
-- to use this option you have to create file called client_assertions.txt and make sure after client crash to change IP to otserv one then click "send".
saveclientdebug = true

-- Set the max query interval for retrieving status information
-- default: 5 minutes / IP
-- set to 0 to disable
statustimeout = 30 * 1000

-- accounts password type
-- options: plain, md5, sha1
passwordtype = "plain"

-- Password salt
-- Offers better security against rainbow table attacks etc.
passwordsalt = ""

-- House configuration (game_class)
-- House tile price
house_tile_price = 100
-- Only premium players can buy houses?
house_only_premium = true
-- Level to buy houses
house_level = 1
-- Show house price and its rent price when looking at its door?
-- PS: This info is going to be shown just if house has no owner
show_house_prices = false

-- SQL type
-- options: mysql, sqlite, odbc or pgsql
sql_type = "sqlite"

-- SQL connection part
sql_db   = "db.s3db"
--  "otserv" -- use this for MySQL/PgSQL
--  "Your Source" -- ODBC

-- Additional connection settings for mysql, pgsql and odbc
sql_host = "127.0.0.1"
sql_port = 3306
-- 5432 -- use this port for PgSQL
sql_user = "root"
sql_pass = ""

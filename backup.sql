CREATE TABLE "accounts" (
    "id" INTEGER  PRIMARY KEY NOT NULL,
    "password" VARCHAR(255) NOT NULL,
    "email" VARCHAR(255) NOT NULL DEFAULT '',
    "blocked" BOOLEAN NOT NULL DEFAULT FALSE,
    "premdays" INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE "groups" (
    "id" INTEGER PRIMARY KEY,
    "name" VARCHAR(255) NOT NULL,
    "flags" INTEGER NOT NULL DEFAULT 0,
    "access" INTEGER NOT NULL,
    "maxdepotitems" INTEGER NOT NULL,
    "maxviplist" INTEGER NOT NULL
);

CREATE TABLE "players" (
    "id" INTEGER  PRIMARY KEY NOT NULL,
    "name" VARCHAR(255) NOT NULL,
    "account_id" INTEGER NOT NULL,
    "group_id" INTEGER NOT NULL,
    "sex" INTEGER NOT NULL DEFAULT 0,
    "vocation" INTEGER NOT NULL DEFAULT 0,
    "experience" INTEGER NOT NULL DEFAULT 0,
    "level" INTEGER NOT NULL DEFAULT 1,
    "maglevel" INTEGER NOT NULL DEFAULT 0,
    "health" INTEGER NOT NULL DEFAULT 100,
    "healthmax" INTEGER NOT NULL DEFAULT 100,
    "mana" INTEGER NOT NULL DEFAULT 100,
    "manamax" INTEGER NOT NULL DEFAULT 100,
    "manaspent" INTEGER NOT NULL DEFAULT 0,
    "soul" INTEGER NOT NULL DEFAULT 0,
    "direction" INTEGER NOT NULL DEFAULT 0,
    "lookbody" INTEGER NOT NULL DEFAULT 10,
    "lookfeet" INTEGER NOT NULL DEFAULT 10,
    "lookhead" INTEGER NOT NULL DEFAULT 10,
    "looklegs" INTEGER NOT NULL DEFAULT 10,
    "looktype" INTEGER NOT NULL DEFAULT 136,
    "lookaddons" INTEGER NOT NULL DEFAULT 0,
    "posx" INTEGER NOT NULL DEFAULT 0,
    "posy" INTEGER NOT NULL DEFAULT 0,
    "posz" INTEGER NOT NULL DEFAULT 0,
    "cap" INTEGER NOT NULL DEFAULT 0,
    "lastlogin" INTEGER NOT NULL DEFAULT 0,
    "lastip" INTEGER NOT NULL DEFAULT 0,
    "save" BOOLEAN NOT NULL DEFAULT TRUE,
    "conditions" BLOB NOT NULL,
    "redskulltime" INTEGER NOT NULL DEFAULT 0,
    "redskull" BOOLEAN NOT NULL DEFAULT FALSE,
    "guildnick" VARCHAR(255) NOT NULL DEFAULT '',
    "rank_id" INTEGER NOT NULL,
    "town_id" INTEGER NOT NULL,
    "loss_experience" INTEGER NOT NULL DEFAULT 10,
    "loss_mana" INTEGER NOT NULL DEFAULT 10,
    "loss_skills" INTEGER NOT NULL DEFAULT 10,
    FOREIGN KEY ("account_id") REFERENCES "accounts" ("id"),
    FOREIGN KEY ("group_id") REFERENCES "groups" ("id")
);

CREATE TABLE "guilds" (
    "id" INTEGER PRIMARY KEY,
    "name" VARCHAR(255) NOT NULL,
    "ownerid" INTEGER NOT NULL,
    "creationdata" INTEGER NOT NULL,
    FOREIGN KEY ("ownerid") REFERENCES "players" ("id")
);

CREATE TABLE "guild_ranks" (
    "id" INTEGER PRIMARY KEY,
    "guild_id" INTEGER NOT NULL,
    "name" VARCHAR(255) NOT NULL,
    "level" INTEGER NOT NULL,
    FOREIGN KEY ("guild_id") REFERENCES "guilds" ("id")
);

CREATE TABLE "houses" (
    "id" INTEGER PRIMARY KEY,
    "owner" INTEGER NOT NULL,
    "paid" INTEGER NOT NULL DEFAULT 0,
    "warnings" TEXT NOT NULL
);

CREATE TABLE "house_lists" (
    "house_id" INTEGER NOT NULL,
    "listid" INTEGER NOT NULL,
    "list" TEXT NOT NULL,
    FOREIGN KEY ("house_id") REFERENCES "houses" ("id")
);

CREATE TABLE "player_depotitems" (
    "player_id" INTEGER NOT NULL,
    "sid" INTEGER NOT NULL,
    "pid" INTEGER NOT NULL DEFAULT 0,
    "itemtype" INTEGER NOT NULL,
    "count" INTEGER NOT NULL DEFAULT 0,
    "attributes" BLOB NOT NULL,
    FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "player_skills" (
    "player_id" INTEGER NOT NULL,
    "skillid" INTEGER NOT NULL,
    "value" INTEGER NOT NULL DEFAULT 0,
    "count" INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "player_storage" (
    "player_id" INTEGER NOT NULL,
    "key" INTEGER NOT NULL,
    "value" INTEGER NOT NULL,
    FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "player_viplist" (
    "player_id" INTEGER NOT NULL,
    "vip_id" INTEGER NOT NULL,
    FOREIGN KEY ("player_id") REFERENCES "players" ("id"),
    FOREIGN KEY ("vip_id") REFERENCES "players" ("id")
);

CREATE TABLE "tiles" (
    "id" INTEGER PRIMARY KEY,
    "x" INTEGER NOT NULL,
    "y" INTEGER NOT NULL,
    "z" INTEGER NOT NULL
);

CREATE TABLE "tile_items" (
    "tile_id" INTEGER NOT NULL,
    "sid" INTEGER NOT NULL,
    "pid" INTEGER NOT NULL DEFAULT 0,
    "itemtype" INTEGER NOT NULL,
    "count" INTEGER NOT NULL DEFAULT 0,
    "attributes" BLOB NOT NULL,
    FOREIGN KEY ("tile_id") REFERENCES "tiles" ("id")
);

CREATE TABLE "player_items" (
    "player_id" INT NOT NULL,
    "sid" INT NOT NULL,
    "pid" INT NOT NULL DEFAULT 0,
    "itemtype" INT NOT NULL,
    "count" INT NOT NULL DEFAULT 0,
    "attributes" BLOB NOT NULL,
    FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "player_spells" (
    "player_id" INTEGER NOT NULL,
    "name" VARCHAR(255) NOT NULL,
    FOREIGN KEY ("player_id") REFERENCES "players" ("id")
);

CREATE TABLE "bans" (
    "type" INTEGER NOT NULL DEFAULT 0,
    "ip" INTEGER NOT NULL DEFAULT 0,
    "mask" INTEGER NOT NULL DEFAULT 4294967295,
    "player" INTEGER NOT NULL DEFAULT 0,
    "account" INTEGER NOT NULL DEFAULT 0,
    "time" INTEGER NOT NULL DEFAULT 0
);

CREATE TRIGGER "oncreate_guilds"
AFTER INSERT ON "guilds"
BEGIN
    INSERT INTO "guild_ranks" ("name", "level", "guild_id") VALUES ("Leader", 3, NEW."id");
    INSERT INTO "guild_ranks" ("name", "level", "guild_id") VALUES ("Vice-Leader", 2, NEW."id");
    INSERT INTO "guild_ranks" ("name", "level", "guild_id") VALUES ("Member", 1, NEW."id");
END;

CREATE TRIGGER "oncreate_players"
AFTER INSERT
ON "players"
BEGIN
    INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 0, 10);
    INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 1, 10);
    INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 2, 10);
    INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 3, 10);
    INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 4, 10);
    INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 5, 10);
    INSERT INTO "player_skills" ("player_id", "skillid", "value") VALUES (NEW."id", 6, 10);
END;

CREATE TRIGGER "ondelete_accounts"
BEFORE DELETE
ON "accounts"
FOR EACH ROW
BEGIN
    DELETE FROM "players" WHERE "account_id" = OLD."id";
    DELETE FROM "bans" WHERE "type" = 3 AND "account" = OLD."id";
END;

CREATE TRIGGER "ondelete_players"
BEFORE DELETE
ON "players"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'DELETE on table "players" violates foreign: "ownerid" from table "guilds"')
    WHERE (SELECT "id" FROM "guilds" WHERE "ownerid" = OLD."id") IS NOT NULL;

    DELETE FROM "player_viplist" WHERE "player_id" = OLD."id" OR "vip_id" = OLD."id";
    DELETE FROM "player_storage" WHERE "player_id" = OLD."id";
    DELETE FROM "player_skills" WHERE "player_id" = OLD."id";
    DELETE FROM "player_items" WHERE "player_id" = OLD."id";
    DELETE FROM "player_depotitems" WHERE "player_id" = OLD."id";
    DELETE FROM "player_spells" WHERE "player_id" = OLD."id";
    DELETE FROM "bans" WHERE "type" = 2 AND "player" = OLD."id";
    UPDATE "houses" SET "owner" = 0 WHERE "owner" = OLD."id";
END;

CREATE TRIGGER "ondelete_guilds"
BEFORE DELETE
ON "guilds"
FOR EACH ROW
BEGIN
    UPDATE "players" SET "guildnick" = '', "rank_id" = 0 WHERE "rank_id" IN (SELECT "id" FROM "guild_ranks" WHERE "guild_id" = OLD."id");
    DELETE FROM "guild_ranks" WHERE "guild_id" = OLD."id";
END;

CREATE TRIGGER "oninsert_players"
BEFORE INSERT
ON "players"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'INSERT on table "players" violates foreign: "account_id"')
    WHERE NEW."account_id" IS NULL
        OR (SELECT "id" FROM "accounts" WHERE "id" = NEW."account_id") IS NULL;

    SELECT RAISE(ROLLBACK, 'INSERT on table "players" violates foreign: "group_id"')
    WHERE NEW."group_id" IS NULL
        OR (SELECT "id" FROM "groups" WHERE "id" = NEW."group_id") IS NULL;
END;

CREATE TRIGGER "onupdate_players"
BEFORE UPDATE
ON "players"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'UPDATE on table "players" violates foreign: "account_id"')
    WHERE NEW."account_id" IS NULL
        OR (SELECT "id" FROM "accounts" WHERE "id" = NEW."account_id") IS NULL;

    SELECT RAISE(ROLLBACK, 'UPDATE on table "players" violates foreign: "group_id"')
    WHERE NEW."group_id" IS NULL
        OR (SELECT "id" FROM "groups" WHERE "id" = NEW."group_id") IS NULL;
END;

CREATE TRIGGER "ondelete_groups"
BEFORE DELETE
ON "groups"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'DELETE on table "guilds" violates foreign: "group_id" from table "players"')
    WHERE (SELECT "id" FROM "players" WHERE "group_id" = OLD."id") IS NOT NULL;
END;

CREATE TRIGGER "oninsert_guilds"
BEFORE INSERT
ON "guilds"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'INSERT on table "guilds" violates foreign: "ownerid"')
    WHERE NEW."ownerid" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."ownerid") IS NULL;
END;

CREATE TRIGGER "onupdate_guilds"
BEFORE UPDATE
ON "guilds"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'UPDATE on table "guilds" violates foreign: "ownerid"')
    WHERE NEW."ownerid" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."ownerid") IS NULL;
END;

CREATE TRIGGER "ondelete_houses"
BEFORE DELETE
ON "houses"
FOR EACH ROW
BEGIN
    DELETE FROM "house_lists" WHERE "house_id" = OLD."id";
END;

CREATE TRIGGER "ondelete_tiles"
BEFORE DELETE
ON "tiles"
FOR EACH ROW
BEGIN
    DELETE FROM "tile_items" WHERE "tile_id" = OLD."id";
END;

CREATE TRIGGER "oninsert_guild_ranks"
BEFORE INSERT
ON "guild_ranks"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'INSERT on table "guild_ranks" violates foreign: "guild_id"')
    WHERE NEW."guild_id" IS NULL
        OR (SELECT "id" FROM "guilds" WHERE "id" = NEW."guild_id") IS NULL;
END;

CREATE TRIGGER "onupdate_guild_ranks"
BEFORE UPDATE
ON "guild_ranks"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'UPDATE on table "guild_ranks" violates foreign: "guild_id"')
    WHERE NEW."guild_id" IS NULL
        OR (SELECT "id" FROM "guilds" WHERE "id" = NEW."guild_id") IS NULL;
END;

CREATE TRIGGER "oninsert_house_lists"
BEFORE INSERT
ON "house_lists"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'INSERT on table "house_lists" violates foreign: "house_id"')
    WHERE NEW."house_id" IS NULL
        OR (SELECT "id" FROM "houses" WHERE "id" = NEW."house_id") IS NULL;
END;

CREATE TRIGGER "onupdate_house_lists"
BEFORE UPDATE
ON "house_lists"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'UPDATE on table "house_lists" violates foreign: "house_id"')
    WHERE NEW."house_id" IS NULL
        OR (SELECT "id" FROM "houses" WHERE "id" = NEW."house_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_depotitems"
BEFORE INSERT
ON "player_depotitems"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'INSERT on table "player_depotitems" violates foreign: "player_id"')
    WHERE NEW."player_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_depotitems"
BEFORE UPDATE
ON "player_depotitems"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'UPDATE on table "player_depotitems" violates foreign: "player_id"')
    WHERE NEW."player_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_skills"
BEFORE INSERT
ON "player_skills"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'INSERT on table "player_skills" violates foreign: "player_id"')
    WHERE NEW."player_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_skills"
BEFORE UPDATE
ON "player_skills"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'UPDATE on table "player_skills" violates foreign: "player_id"')
    WHERE NEW."player_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_storage"
BEFORE INSERT
ON "player_storage"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'INSERT on table "player_storage" violates foreign: "player_id"')
    WHERE NEW."player_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_storage"
BEFORE UPDATE
ON "player_storage"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'UPDATE on table "player_storage" violates foreign: "player_id"')
    WHERE NEW."player_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_viplist"
BEFORE INSERT
ON "player_viplist"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'INSERT on table "player_viplist" violates foreign: "player_id"')
    WHERE NEW."player_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;

    SELECT RAISE(ROLLBACK, 'INSERT on table "player_viplist" violates foreign: "vip_id"')
    WHERE NEW."vip_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."vip_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_viplist"
BEFORE UPDATE
ON "player_viplist"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'UPDATE on table "player_viplist" violates foreign: "vip_id"')
    WHERE NEW."vip_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."vip_id") IS NULL;
END;

CREATE TRIGGER "oninsert_tile_items"
BEFORE INSERT
ON "tile_items"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'INSERT on table "tile_items" violates foreign: "tile_id"')
    WHERE NEW."tile_id" IS NULL
        OR (SELECT "id" FROM "tiles" WHERE "id" = NEW."tile_id") IS NULL;
END;

CREATE TRIGGER "onupdate_tile_items"
BEFORE UPDATE
ON "tile_items"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'UPDATE on table "tile_items" violates foreign: "tile_id"')
    WHERE NEW."tile_id" IS NULL
        OR (SELECT "id" FROM "tiles" WHERE "id" = NEW."tile_id") IS NULL;
END;

CREATE TRIGGER "oninsert_player_spells"
BEFORE INSERT
ON "player_spells"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'INSERT on table "player_spells" violates foreign: "player_id"')
    WHERE NEW."player_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

CREATE TRIGGER "onupdate_player_spells"
BEFORE UPDATE
ON "player_spells"
FOR EACH ROW
BEGIN
    SELECT RAISE(ROLLBACK, 'UPDATE on table "player_spells" violates foreign: "player_id"')
    WHERE NEW."player_id" IS NULL
        OR (SELECT "id" FROM "players" WHERE "id" = NEW."player_id") IS NULL;
END;

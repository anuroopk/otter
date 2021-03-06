CREATE TABLE "visits" ("id" INTEGER PRIMARY KEY, "location" INTEGER NOT NULL, "icon" INTEGER NOT NULL, "title" TEXT, "time" INTEGER NOT NULL, "typed" BOOLEAN NOT NULL);
CREATE TABLE "locations" ("id" INTEGER PRIMARY KEY, "host" INTEGER NOT NULL, "scheme" TEXT NOT NULL, "path" TEXT, UNIQUE("host", "scheme", "path"));
CREATE TABLE "hosts" ("id" INTEGER PRIMARY KEY, "host" TEXT UNIQUE NOT NULL);
CREATE TABLE "icons" ("id" INTEGER PRIMARY KEY, "icon" BLOB UNIQUE NOT NULL);

-- spanisch → deutsch
CREATE TABLE IF NOT EXISTS spanisch_deutsch_erster_versuch (
  id      INTEGER PRIMARY KEY AUTOINCREMENT,
  span    TEXT NOT NULL,
  deut    TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS spanisch_deutsch_zweiter_versuch (
  id      INTEGER PRIMARY KEY AUTOINCREMENT,
  span    TEXT NOT NULL,
  deut    TEXT NOT NULL
);

-- deutsch → spanisch
CREATE TABLE IF NOT EXISTS deutsch_spanisch_erster_versuch (
  id      INTEGER PRIMARY KEY AUTOINCREMENT,
  deut    TEXT NOT NULL,
  span    TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS deutsch_spanisch_zweiter_versuch (
  id      INTEGER PRIMARY KEY AUTOINCREMENT,
  deut    TEXT NOT NULL,
  span    TEXT NOT NULL
);

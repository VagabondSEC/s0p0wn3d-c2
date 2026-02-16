CREATE TABLE "MorphinAgent" (
    "agent_id" varchar(64) NOT NULL PRIMARY KEY,
    "shared_secret" blob NULL,
    "hostname" varchar(255) NOT NULL,
    "username" varchar(255) NOT NULL,
    "is_admin" bool NOT NULL,
    "domain" varchar(255) NOT NULL,
    "os_version" varchar(255) NOT NULL,
    "architecture" varchar(10) NOT NULL,
    "process_id" integer NULL,
    "ip_addresses" text NOT NULL,
    "last_seen" datetime NOT NULL,
    "jitter_base" integer NOT NULL,
    "is_active" bool NOT NULL
);

CREATE TABLE "CommandQueue" (
    "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "command_text" varchar(255) NOT NULL,
    "command_value" integer NOT NULL,
    "is_sent" bool NOT NULL,
    "created_at" datetime NOT NULL,
    "agent_id" varchar(64) NOT NULL REFERENCES "MorphinAgent" ("agent_id")
);
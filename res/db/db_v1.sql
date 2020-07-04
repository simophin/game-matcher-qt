create table settings
(
    name  text not null primary key,
    value text not null
);
---
insert into settings(name, value)
values ('schema_version', 1);
---
create table members
(
    id           integer  not null primary key autoincrement,
    registerDate datetime not null default current_timestamp,
    firstName    text     not null,
    lastName     text     not null,
    gender       text     not null,
    level        integer  not null,
    email        text,
    phone        text,
    check ( gender = 'male' or gender = 'female' ),
    check ( level >= 0 and level <= 10 )
);
---
create unique index member_unique_names on members (firstName, lastName);
---
create table courts
(
    id        integer not null primary key autoincrement,
    sessionId integer not null references sessions (id) on delete cascade,
    name      text    not null,
    sortOrder integer not null
);
---
create unique index courts_session_order on courts (sessionId, sortOrder);
---
create index courts_sessions on courts (sessionId);
---
create table sessions
(
    id                 integer  not null primary key autoincrement,
    startTime          datetime not null default current_timestamp,
    fee                integer  not null,
    numPlayersPerCourt integer  not null,
    place              text     not null,
    announcement       text,
    check ( fee >= 0 )
);
---
create table players
(
    id           integer  not null primary key autoincrement,
    sessionId    integer  not null references sessions (id) on delete cascade,
    memberId     integer  not null references members (id) on delete cascade,
    paid         boolean  not null,
    checkInTime  datetime not null default current_timestamp,
    checkOutTime datetime,
    paused       boolean  not null default false,
    unique (sessionId, memberId) on conflict fail
);
---
create index player_sessions on players (sessionId);
---
create index player_members on players (memberId);
---
create table games
(
    id        integer  not null primary key autoincrement,
    sessionId integer  not null references sessions (id) on delete cascade,
    startTime datetime not null default current_timestamp
);
---
create index game_sessions on games (sessionId);
---
create table game_allocations
(
    gameId   integer not null references games (id) on delete cascade,
    courtId  integer not null references courts (id) on delete cascade,
    playerId integer not null references players (id) on delete cascade,
    primary key (gameId, courtId, playerId),
    unique (gameId, playerId) on conflict fail
);
---
create index game_allocations_games on game_allocations (gameId);
---
create index game_allocations_courts on game_allocations (courtId);
---
create index game_allocations_players on game_allocations (playerId);

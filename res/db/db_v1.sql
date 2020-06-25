create table settings
(
    name text not null primary key,
    value text not null
);

insert into settings(name, value) values ('schema_version', 1);

create table members
(
    id              integer  not null primary key autoincrement,
    register_date   datetime not null default current_timestamp,
    first_name      text     not null,
    last_name       text     not null,
    gender          text     not null,
    level           integer  not null,
    initial_balance integer  not null default 0,
    check ( gender = 'male' or gender = 'female' ),
    check ( level >= 0 and level <= 10 )
);


create table courts
(
    id         integer not null primary key autoincrement,
    session_id integer not null references sessions (id) on delete cascade,
    name       text    not null,
    sort_order integer not null
);

create index courts_sessions on courts (session_id);

create table sessions
(
    id           integer  not null primary key autoincrement,
    start_time   datetime not null default current_timestamp,
    fee          integer  not null,
    announcement text
    check ( fee >= 0 )
);

create table players
(
    id             integer  not null primary key autoincrement,
    session_id     integer  not null references sessions (id) on delete cascade,
    member_id      integer  not null references members (id) on delete cascade,
    payment        integer  not null,
    check_in_time  datetime not null default current_timestamp,
    check_out_time datetime,
    paused         boolean  not null default false,
    unique (session_id, member_id) on conflict fail
);

create index player_sessions on players (session_id);
create index player_members on players (member_id);

create table games
(
    id         integer  not null primary key autoincrement,
    session_id integer  not null references sessions (id) on delete cascade,
    start_time datetime not null default current_timestamp
);

create index game_sessions on games (session_id);

create table game_allocations
(
    game_id   integer not null references games (id) on delete cascade,
    court_id  integer not null references courts (id) on delete cascade,
    player_id integer not null references players (id) on delete cascade,
    primary key (game_id, court_id, player_id),
    unique (game_id, player_id) on conflict fail,
    unique (court_id, player_id) on conflict fail
);

create index game_allocations_games on game_allocations (game_id);
create index game_allocations_courts on game_allocations (court_id);
create index game_allocations_players on game_allocations (player_id);

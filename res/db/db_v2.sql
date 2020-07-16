update settings set value = 2 where name = 'schema_version';
---
alter table game_allocations add column quality integer default 0;

update settings
set value = 3
where name = 'schema_version';
---
create trigger check_member_duplication_i
    before insert
    on members
begin
    select case
               when count(M.id) > 0 THEN
                   raise(abort, 'Member with the same name exists')
               end
    from members M
    where trim(M.firstName) == trim(new.firstName) collate nocase
      and trim(M.lastName) == trim(new.lastName) collate nocase;
end;
---
create trigger check_member_duplication_u
    before update
    on members
begin
    select case
               when count(M.id) > 0 THEN
                   raise(abort, 'Member with the same name exists')
               end
    from members M
    where trim(M.firstName) == trim(new.firstName) collate nocase
      and trim(M.lastName) == trim(new.lastName) collate nocase
      and M.id != old.id;
end;

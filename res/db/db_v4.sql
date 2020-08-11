update settings
set value = 4
where name = 'schema_version';
---
create view normalized_members as
select M.id,
       cast(strftime('%s', M.registerDate) as INTEGER) as registerDate,
       M.firstName,
       M.lastName,
       M.gender,
       M.level from members M;
---
create view session_members as
select M.*,
       p.paid,
       (case
            when (p.checkOutTime is not null) then 'CheckedOut'
            when p.paused then 'CheckedInPaused'
            else 'CheckedIn'
           end)                                        as status,
       p.sessionId
from players p
         inner join normalized_members m on m.id = p.memberId;
---
create view checked_in_non_paused_members as
select M.*,
       p.paid,
       'CheckedIn' as status,
       p.sessionId
from players p
         inner join normalized_members m on m.id = p.memberId
where not p.paused and p.checkOutTime is null;
---
create view paused_members as
select M.*,
       p.paid,
       'CheckedInPaused' as status,
       p.sessionId
from players p
         inner join normalized_members m on m.id = p.memberId
where p.paused and p.checkOutTime is null;
---
create view unchecked_in_members as
select M.*,
       false as paid,
       'NotCheckedIn' as status,
       S.id as sessionId
from normalized_members M
         left join sessions S
where M.id not in (select P.memberId from players P where P.sessionId = S.id)

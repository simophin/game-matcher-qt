update settings set value = 2 where name = 'schema_version';
---
create view member_session_info as
    select M.*, P.sessionId, count(GA.gameId) as numGames
    from members M
    inner join players P on P.memberId = M.id
    left join game_allocations GA on GA.playerId = P.id
    group by M.id, P.sessionId
CREATE or replace view view_locks as
SELECT
    relation::regclass,
    mode,
    locktype,
    -- 虚拟事务 ID
    virtualxid AS v_tid,
    -- 事务 ID
    transactionid AS tid,
    -- 没有事务在请求这把锁，该字段就是正在持有这把锁的虚拟事务 ID
    -- 有事务在请求这把锁，该字段就是正在请求这把锁的虚拟事务 ID
    virtualtransaction AS blocked_v_tid,
    -- 当前事务拥有这把锁，该值就是 true,
    -- 当前事务不拥有这把锁，改值就是 false
    granted
FROM
    pg_locks;


CREATE OR REPLACE FUNCTION show_locks() RETURNS SETOF record AS
$$
    select * from view_locks;
$$ LANGUAGE SQL;

SELECT pg_stat_activity.pid, pg_stat_activity.query
FROM pg_locks
JOIN pg_stat_activity ON pg_stat_activity.pid = pg_locks.pid
WHERE pg_locks.virtualtransaction = '4/255';

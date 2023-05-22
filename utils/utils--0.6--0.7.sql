-- drop it created by 0.6 version
drop view view_locks;

CREATE VIEW view_locks AS
SELECT  pid, virtualtransaction AS vxid, locktype AS lock_type, 
        mode AS lock_mode, granted,
        CASE
                WHEN virtualxid IS NOT NULL AND transactionid 
IS NOT NULL
                THEN    virtualxid || ' ' || transactionid
                WHEN virtualxid::text IS NOT NULL
                THEN    virtualxid
                ELSE    transactionid::text
        END AS xid_lock, relname,
        page, tuple, classid, objid, objsubid
FROM    pg_locks LEFT OUTER JOIN pg_class ON (pg_locks.relation 
= pg_class.oid)
WHERE   -- do not show our view's locks
        pid != pg_backend_pid() AND
        -- no need to show self-vxid locks
        virtualtransaction IS DISTINCT FROM virtualxid
-- granted is ordered earlier
ORDER BY 1, 2, 5 DESC, 6, 3, 4, 7;

CREATE VIEW view_locks1 AS SELECT pid, vxid, lock_type, lock_mode, granted, xid_lock, relname FROM view_locks ORDER BY 1, 2, 5 DESC, 6, 3, 4, 7;

CREATE VIEW view_locks2 AS SELECT pid, vxid, lock_type, page, tuple, classid, objid, objsubid FROM view_locks
-- granted is first
-- add non-display columns to match ordering of lockview
ORDER BY 1, 2, granted DESC, vxid, xid_lock::text, 3, 4, 5, 6, 7, 8;

-- cannot be a temporary view because other sessions must see it
CREATE VIEW view_lock_stat AS
SELECT pg_stat_activity.pid AS pid,
query, wait_event, vxid, lock_type,
lock_mode, granted, xid_lock
FROM view_locks JOIN pg_stat_activity ON (view_locks.pid = pg_stat_activity.pid);


CREATE VIEW lockinfo_hierarchy AS
        WITH RECURSIVE lockinfo1 AS (
                SELECT pid, vxid, granted, xid_lock, lock_type, 
relname, page, tuple
                FROM view_locks
                WHERE xid_lock IS NOT NULL AND
                      relname IS NULL AND
                      granted
                UNION ALL
                SELECT lockview.pid, lockview.vxid, 
lockview.granted, lockview.xid_lock, 
                        lockview.lock_type, lockview.relname, 
lockview.page, lockview.tuple
                FROM lockinfo1 JOIN lockview ON 
(lockinfo1.xid_lock = lockview.xid_lock)
                WHERE lockview.xid_lock IS NOT NULL AND
                      lockview.relname IS NULL AND
                      NOT lockview.granted AND
                      lockinfo1.granted),
        lockinfo2 AS (
                SELECT pid, vxid, granted, xid_lock, lock_type, 
relname, page, tuple
                FROM lockview
                WHERE lock_type = 'tuple' AND
                      granted
                UNION ALL
                SELECT lockview.pid, lockview.vxid, 
lockview.granted, lockview.xid_lock,
                        lockview.lock_type, lockview.relname, 
lockview.page, lockview.tuple
                FROM lockinfo2 JOIN lockview ON (
                        lockinfo2.lock_type = 
lockview.lock_type AND
                        lockinfo2.relname = lockview.relname 
AND
                        lockinfo2.page = lockview.page AND
                        lockinfo2.tuple = lockview.tuple)
                WHERE lockview.lock_type = 'tuple' AND
                      NOT lockview.granted AND
                      lockinfo2.granted
        )
        SELECT * FROM lockinfo1
        UNION ALL
        SELECT * FROM lockinfo2;
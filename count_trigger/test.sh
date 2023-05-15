#!/bin/bash

echo "
INSERT INTO ttest VALUES (NULL);
SELECT * FROM ttest;
INSERT INTO ttest VALUES (1);
SELECT * FROM ttest;
INSERT INTO ttest SELECT x * 2 FROM ttest;
SELECT * FROM ttest;
UPDATE ttest SET x = NULL WHERE x = 2;
SELECT * FROM ttest;
DELETE FROM ttest;
SELECT * FROM ttest;
" \
| psql mydb
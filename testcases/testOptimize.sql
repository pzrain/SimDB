CREATE DATABASE testOptimize;
USE testOptimize;
CREATE TABLE test(col1 INT, col2 INT);
ALTER TABLE test ADD INDEX (col1);
INSERT INTO test VALUES (1, 2), (3, 2), (4, 4);
SHOW INDEXES;
SELECT * FROM test WHERE test.col1 >= test.col2;
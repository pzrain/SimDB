CREATE DATABASE unique;
USE unique;
CREATE TABLE test(id INT);
INSERT INTO test VALUES(0), (1), (2), (3), (4), (5), (6), (7), (8), (9), (10), (11), (12), (13), (14), (15);
ALTER TABLE test ADD UNIQUE (id);
INSERT INTO test VALUES(9);
ALTER TABLE test DROP UNIQUE (id);
INSERT INTO test VALUES(9);
SELECT COUNT(*) FROM test;
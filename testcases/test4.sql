CREATE DATABASE mysql;
USE mysql;
CREATE TABLE test_table(test_int INT, test_float FLOAT DEFAULT 2.33, test_str VARCHAR(8));
INSERT INTO test_table VALUES (3, 3.33,'testst');
SELECT * FROM test_table;
UPDATE test_table SET test_int=1, test_str='newnew' WHERE test_float = 3.33;
SELECT * FROM test_table;
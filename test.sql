CREATE DATABASE mysql;
USE mysql;
CREATE TABLE test_table(test_int INT, test_float FLOAT, test_str VARCHAR(8));
DESC test_table;
INSERT INTO test_table VALUES (3, 3.33, 'test');
SELECT * FROM test_table;

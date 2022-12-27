CREATE DATABASE mysql;
USE mysql;

CREATE TABLE test_table(test_int INT DEFAULT 2, 
                        test_float FLOAT DEFAULT 2.33, 
                        test_str VARCHAR(8));

DESC test_table;

INSERT INTO test_table 
VALUES (3, 3.33, 'test'), (7, 6.23, 'new'), (527, 8.7861, 'old');



SELECT * FROM test_table;

SELECT MAX(test_int) FROM test_table;
-- SELECT MIN(test_int) FROM test_table;
-- SELECT SUM(test_int) FROM test_table;
-- SELECT AVG(test_int) FROM test_table;


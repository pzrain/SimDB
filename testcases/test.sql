CREATE DATABASE mysql;
USE mysql;

CREATE TABLE test_table(test_int INT DEFAULT 2, 
                        test_float FLOAT DEFAULT 2.33, 
                        test_str VARCHAR(8));

DESC test_table;

INSERT INTO test_table 
VALUES (3, 3.33, 'test'), (7, 6.23, 'new'), (527, 8.7861, 'old');

UPDATE test_table
SET test_int = 777, test_float = 3.14159
WHERE test_str = 'test';


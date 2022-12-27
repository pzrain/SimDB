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

SELECT * FROM test_table;

SELECT test_int, test_str FROM test_table;



SELECT * FROM test_table 
WHERE test_table.test_float > 3.33;

SELECT * FROM test_table 
WHERE test_table.test_float < 7.33;

SELECT * FROM test_table 
WHERE test_table.test_str = 'test';

SELECT * FROM test_table 
WHERE test_table.test_float <= 6.33;

SELECT * FROM test_table 
WHERE test_table.test_int >= 7;

SELECT * FROM test_table 
WHERE test_table.test_float <> 3.33;



SELECT test_int FROM test_table
WHERE test_int = 527;

SELECT * FROM test_table
WHERE test_table.test_int = (
    SELECT test_int FROM test_table
    WHERE test_int = 527
);



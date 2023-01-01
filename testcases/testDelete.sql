CREATE DATABASE test_delete;
USE test_delete;

CREATE TABLE test_table(test_int INT, 
                        test_float FLOAT, 
                        test_str VARCHAR(32));

INSERT INTO test_table VALUES (1, 2.33, 'abc'), (3, 6.66, 'in');

SELECT * FROM test_table;

DELETE FROM test_table WHERE test_int = 3;

SELECT * FROM test_table;
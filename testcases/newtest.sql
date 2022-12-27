USE mysql;
SELECT * FROM test_table;

UPDATE test_table
SET test_int = 1, test_str = 'newtest'
WHERE test_float = 3.33;


SELECT * FROM test_table;

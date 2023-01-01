CREATE DATABASE test6;
USE test6;
CREATE TABLE table_1(id INT);
CREATE TABLE table_2(id INT);
ALTER TABLE table_1 ADD INDEX (id);
INSERT INTO table_1 VALUES(1), (2), (1), (2);
INSERT INTO table_2 VALUES(1), (2);
SELECT * FROM table_1, table_2 WHERE table_2.id=table_1.id;

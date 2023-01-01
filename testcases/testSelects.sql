CREATE DATABASE test_select;
USE test_select;

CREATE TABLE test(
    id INT,
    website VARCHAR(32),
    user INT 
);

INSERT INTO test 
VALUES (1, 'www.bilibili.com', 114514), 
       (2, 'www.netflix.com', 123456), 
       (3, 'www.youtube.com', 336542),
       (4, 'www.nicovideo.jp', 114514);

CREATE TABLE access(
    site_id INT,
    daily_access INT
);

INSERT INTO access
VALUES (1, 299),
       (1, 33),
       (4, 532),
       (4, 665),
       (2, 1232),
       (3, 3322),
       (2, 666);

SELECT * FROM test;

SELECT * FROM test WHERE id > 1 AND user = 114514;

SELECT * FROM test WHERE id = (SELECT * FROM test WHERE user = 123456);

SELECT * FROM test WHERE id IS NOT NULL;

SELECT * FROM test WHERE id IN (3, 4);

SELECT * FROM test WHERE website LIKE '%.com';

SELECT * FROM test WHERE id IN (SELECT * FROM test WHERE website LIKE 'www.[a-z]{7}.com');

SELECT test.website FROM test WHERE user = 114514;

SELECT SUM(user) FROM test;

SELECT AVG(user) FROM test;

SELECT MAX(user) FROM test;

SELECT MIN(user) FROM test;

SELECT COUNT(user) FROM test;

SELECT COUNT(*) FROM test;

SELECT access.site_id, SUM(access.daily_access)
FROM access
GROUP BY access.site_id;

SELECT MIN(daily_access) FROM access LIMIT 5;

SELECT MIN(daily_access) FROM access LIMIT 5 OFFSET 2;




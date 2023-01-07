CREATE DATABASE testLike;
USE testLike;

CREATE TABLE test(site VARCHAR(32));
INSERT INTO test VALUES ('google'), ('baidu'), ('bilibili'), ('youtube');
SELECT * FROM test;

SELECT * FROM test WHERE site LIKE 'b%';

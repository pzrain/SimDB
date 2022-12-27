import argparse
ARG = argparse.ArgumentParser()
ARG.add_argument("--file", type=str, required=True)
ARG = ARG.parse_args()

id = 0
with open(ARG.file, 'w') as f:
    f.write("CREATE DATABASE testMoreIndex;\n")
    f.write("USE testMoreIndex;\n")
    f.write("CREATE TABLE test (id INT);\n")
    com = "INSERT INTO test VALUES"
    for id in range(50000):
        com += " (%d)," % (id)
    com = com[:-1]
    com += ";\n"
    f.write(com)
    # f.write("ALTER TABLE test ADD INDEX (id);\n")

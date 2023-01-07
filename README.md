## SimDB
SimDB is the final project for course Introduction to Database in Tsinghua University, Fall 2022. It is a demo-level relational database system. We implement many of the basic SQL statements. It supports both command line and batch mode.

For more details and specific instructions, please refer to `report.pdf` under `doc/`.
### Environment
Tested under Ubuntu20.04 and wsl under windows. To compile, the runtime environment of antlr4 needs to be configured correctly.
### Quick start
```bash
./run.sh
```

compile & run

```bash
./run.sh -c
```

### References

Stanford rebase project: https://web.stanford.edu/class/cs346/2015/redbase.html

Database by YuHao Zhou: https://github.com/miskcoo/TrivialDB

Implementation of B+ tree: https://en.wikipedia.org/wiki/B%2B_tree

Course documentation: https://thu-db.github.io/dbs-tutorial/


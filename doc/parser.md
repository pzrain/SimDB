## <center>SQL解析</center>

### 0.antlr4配置

#### 0.1 Java on Linux

执行以下命令：

```bash
sudo apt update
sudo apt install openjdk-11-jdk
```

完成后`java --version`不会报错

#### 0.2 Download antlr4

在官网下载`antlr-4.11.1-complete.jar`，放置在`/usr/local/lib`目录下，然后在任一其他目录下执行以下命令：

```bash
git clone git@github.com:antlr/antlr4.git
cd antlr4/runtime/Cpp
mkdir build && cd build
cmake .. -DANTLR_JAR_LOCATION=/usr/local/lib/antlr-4.11.1-complete.jar -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_DEMO=True
make
sudo make install
```

完成后应该能在`/usr/local/include`下找到`antlr4-runtime`目录，以及在`/usr/local/lib`下找到`antrl4`的运行时链接库`libantlr4-runtime.so`

#### 0.3 Check

切换到`parser-dev`，在项目根目录下运行`./run.sh`，编译通过，检查项目根目录下的`msg.debug`，可以看到一些输出。

### 1.部分接口说明

### MyErrorListener

`src/parser/MySQLVisitor.h`

```C++
class MyANTLRParserErrorListener : public ANTLRErrorListener {
private:
    int* errorNum;
public:

    MyANTLRParserErrorListener(int* errorNum_): errorNum(errorNum_) {}

    void syntaxError(Recognizer *recognizer, Token *offendingSymbol, size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override {
        printf("[Parser Error] line %ld,%ld %s\n", line, charPositionInLine, msg.c_str());
        (*errorNum)++;
    }

    void reportAmbiguity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, atn::ATNConfigSet *configs) override {}

    void reportAttemptingFullContext(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, const antlrcpp::BitSet &conflictingAlts, atn::ATNConfigSet *configs) override {}

    void reportContextSensitivity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, size_t prediction, atn::ATNConfigSet *configs) override {}
};
```

继承了`ANTLRErrorListener`，在此基础上重写了处理`syntaxError`的函数。这样，程序在`parse`过程中如果遇到语法错误，就会报错并退出此次解析，不会再进入后续对语法树的遍历。

### MySQLVisitor

`src/parser/MySQLVisitor.h`

继承了`SQLBaseVisitor`，在其中可重写不同的`visit`方法，现有的`MySQLVisitor`给出了几个示例，与编原实验非常相似。

实现的时候需要参考`src/parser/antlr4/SQL.g4`这一文法文件，并调用系统管理中的各种接口。

现有的实现方法，是直接遍历抽象语法树，一边解析一边实现。更优化的实现，参考实验文档，是对抽象语法树再进行一系列的操作，得到更优的逻辑计划树乃至物理执行计划，可以在数据库各模块全部完成后再进行考虑。

### 2.对抽象语法树的优化

目前实现了对于多表联合查询的优化。基本原理是基于索引加速。

具体来说，对于形如`WHERE table_3.id = table_2.id AND table_2.id = table_1.id`，如果`table_2.id`与`table_3.id`有建立的索引，那么将查询表达式变形为`WHERE table_1.id = table_2.id AND table_2.id = table_3.id`，查询时先查询`table_1.id`，然后利用索引分别查询`table_2.id`以及`table_3.id`，就可以起到加速效果。另外，对于重复的查询，例如`WHERE table_1.id = table_2.id AND table_2.id = table_3.id AND table_3.id = table_1.id`，第三个表达式是蕴含在前两个中的，其就会被优化掉。

目前实现的形式是**暂时的**，待DBMS模块与多表查询相关的部分完成后，可能需要修改设计。
## <center>SQL解析</center>

### 0.antlr4配置

#### 0.1 Java on Linux

```bash
sudo apt update
sudo apt install openjdk-11-jdk
```

完成后`java --version`不会报错

#### 0.2 Download antlr4

在官网下载`antlr-4.11.1-complete.jar`，放置在`/usr/local/lib`目录下，然后在任一其他目录下

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

切换到`parser-dev`，在项目根目录下运行`./run.sh`，程序正常返回。
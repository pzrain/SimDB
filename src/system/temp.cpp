#include <iostream>
#include <string>
#include <regex>

int main() {
    if (std::regex_match("sub2ject6", std::regex("(sub[1-9]).*"))) {
        std::cout << "***" << std::endl;
    }
    return 0;
}
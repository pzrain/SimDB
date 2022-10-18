#include "RMComponent.h"
#include "../filesystem/utils/pagedef.h"
#include "cstring"

void loadTableHeader(BufType dataLoad, TableHeader* tableHeader) {
    memcpy((uint8_t*)tableHeader, (uint8_t*)dataLoad, sizeof(TableHeader));
    // tableHeader = (TableHeader*)((uint8_t*)dataLoad[sizeof(TableHeader)]);
}

void writeTableHeader(BufType dataLoad, TableHeader* tableHeader) {
    memcpy((uint8_t*)dataLoad, (uint8_t*)tableHeader, sizeof(TableHeader));
}
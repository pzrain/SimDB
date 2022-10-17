#ifndef __RMCOMPONENT_H_
#define __RMCOMPONENT_H_

class Record{
private:
    int pageNumber, slotNumber;
public:
    Record(int pageNumber_, int slotNumber_) {
        pageNumber = pageNumber_;
        slotNumber = slotNumber_;
    }

    ~Record() {}

    int getPageNumber() {
        return pageNumber;
    }

    int getSlotNumber() {
        return slotNumber;
    }
};

#endif
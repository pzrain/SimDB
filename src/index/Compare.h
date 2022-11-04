#ifndef __COMPARE_H__
#define __COMPARE_H__
#include <cmath>
#include <cstring>

class Compare {
public:
    virtual ~Compare() {}
    virtual bool gt(void*, void*) = 0;      // >
    virtual bool gte(void*, void*) = 0;     // >=
    virtual bool lt(void*, void*) = 0;      // <
    virtual bool lte(void*, void*) = 0;     // <=
    virtual bool equ(void*, void*) = 0;   // =
};

class IntCompare: public Compare{
public:
    ~IntCompare() {}

    bool gt(void* aa, void* bb) override {
        int* a = (int*)aa;
        int* b = (int*)bb;
        return (*a) > (*b);
    }

    bool gte(void* aa, void* bb) override {
        int* a = (int*)aa;
        int* b = (int*)bb;
        return (*a) >= (*b);
    }

    bool lt(void* aa, void* bb) override {
        int* a = (int*)aa;
        int* b = (int*)bb;
        return (*a) < (*b);
    }

    bool lte(void* aa, void* bb) override {
        int* a = (int*)aa;
        int* b = (int*)bb;
        return (*a) <= (*b);
    }

    bool equ(void* aa, void* bb) override {
        int* a = (int*)aa;
        int* b = (int*)bb;
        return (*a) == (*b);
    }
};

class FloatCompare: public Compare{
public:
    #define FLOAT_COMPARE_EPSILON 1e-5

    ~FloatCompare() {}

    bool gt(void* aa, void* bb) override {
        float* a = (float*)aa;
        float* b = (float*)bb;
        return (*a) > (*b);
    }

    bool gte(void* aa, void* bb) override {
        float* a = (float*)aa;
        float* b = (float*)bb;
        return (*a) >= (*b);
    }

    bool lt(void* aa, void* bb) override {
        float* a = (float*)aa;
        float* b = (float*)bb;
        return (*a) < (*b);
    }

    bool lte(void* aa, void* bb) override {
        float* a = (float*)aa;
        float* b = (float*)bb;
        return (*a) <= (*b);
    }

    bool equ(void* aa, void* bb) override {
        float* a = (float*)aa;
        float* b = (float*)bb;
        return fabs((*a) - (*b)) < FLOAT_COMPARE_EPSILON;
    }
};

class CharCompare: public Compare{
public:
    ~CharCompare() {}

    bool gt(void* aa, void* bb) override {
        char* a = (char*)aa;
        char* b = (char*)bb;
        return strcmp(a, b) > 0;
    }

    bool gte(void* aa, void* bb) override {
        char* a = (char*)aa;
        char* b = (char*)bb;
        return strcmp(a, b) >= 0;
    }

    bool lt(void* aa, void* bb) override {
        char* a = (char*)aa;
        char* b = (char*)bb;
        return strcmp(a, b) < 0;
    }

    bool lte(void* aa, void* bb) override {
        char* a = (char*)aa;
        char* b = (char*)bb;
        return strcmp(a, b) <= 0;
    }

    bool equ(void* aa, void* bb) override {
        char* a = (char*)aa;
        char* b = (char*)bb;
        return strcmp(a, b) == 0;
    }
};

#endif
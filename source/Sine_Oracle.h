#ifndef _SINE_ORACLE_H_
#define _SINE_ORACLE_H_
#include "Oracle.h"

class Sine_Oracle : public Oracle
{
public: 
    Sine_Oracle(const char* filename);
    void Eval(DataVec& inputDataVec);
};

#endif

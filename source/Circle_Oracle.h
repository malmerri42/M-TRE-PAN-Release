#ifndef _CIRCLE_ORACLE_H_
#define _CIRCLE_ORACLE_H_
#include "Oracle.h"

class Circle_Oracle : public Oracle
{
public:
    Circle_Oracle(const char* filename);
    void Eval(DataVec& inputDataVec);
    std::shared_ptr<DataVec> MarginFilter(const DataVec& inputDataVec, double margin) const;
};

#endif

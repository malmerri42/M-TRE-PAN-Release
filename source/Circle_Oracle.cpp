#include "Circle_Oracle.h"
#include <cmath>
#define RADIUS 1

//default constructor
Circle_Oracle::Circle_Oracle(const char* filename):
    Oracle::Oracle(filename, 0,0)
{
    Oracle::trainingData_.zeros(1000,2);
    Oracle::trainingData_ = (arma::randu(1000,2)*6) - 3;
    Oracle::trainingLabel_.zeros(1000,1);
};

//virtual, implemented by children
void Circle_Oracle::Eval(DataVec& inputDataVec)
{
    //for debugging purposes implemented as a hypersphere of radius 1 centered at origin
    DataVec::iterator inputIter = inputDataVec.begin();
    for(;inputIter != inputDataVec.end(); inputIter++)
    {
        arma::Row<double> tempRow = inputIter->first;
        tempRow = tempRow % tempRow;
        double dist = std::sqrt(arma::accu(tempRow));
        if(dist <= RADIUS)
        {
            inputIter->second = 1;
        }else
        {
            inputIter->second = -1;
        }
    }

}

std::shared_ptr<DataVec> Circle_Oracle::MarginFilter(const DataVec& inputDataVec, double margin) const
{
    //Filter all by placing examples within the margin in the data holder
    std::shared_ptr<DataVec> marginDataPtr = std::make_shared<DataVec>();
    DataVec::const_iterator dataIter = inputDataVec.begin();
    for(; dataIter != inputDataVec.end(); dataIter++)
    {
        arma::Row<double> tempRow = dataIter->first;
        tempRow = tempRow % tempRow;
        double dist = std::sqrt(arma::accu(tempRow));
        if(dist <= RADIUS)
        {
            marginDataPtr->push_back(*dataIter);
        }


    }
    return marginDataPtr;
}

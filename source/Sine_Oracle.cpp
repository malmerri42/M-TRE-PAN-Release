#include "Sine_Oracle.h"
#include <cmath>

//default constructor
Sine_Oracle::Sine_Oracle(const char* filename):
    Oracle::Oracle(filename, 0,0)
{
    Oracle::trainingData_.zeros(1000,2);
    Oracle::trainingData_ = (arma::randu(1000,2)*6) - 3;
    Oracle::trainingLabel_.zeros(1000,1);
};

//virtual, implemented by children
//Sine function, where every data point below the function is negative, and above is positive
void Sine_Oracle::Eval(DataVec& inputDataVec)
{
    //for debugging purposes implemented as a sine wave along the x axis
    DataVec::iterator inputIter = inputDataVec.begin();
    for(;inputIter != inputDataVec.end(); inputIter++)
    {
        arma::Row<double> tempRow = inputIter->first;
        double yVal = std::sin(tempRow(0));
        if(tempRow(1) >  yVal)
        {
            inputIter->second = 1;
        }else
        {
            inputIter->second = -1;
        }
    }

}

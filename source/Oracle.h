#ifndef _ORACLE_
#define _ORACLE_

#include "Constraint.h"

//Parent class of ML algorithm facades 

class Oracle 
{
public:
    //train oracle, and reserve a portion of data for validation
    Oracle(const char* filename, double valiPercent, bool classBalance = true);
    virtual void Eval(DataVec& inputDataVec) = 0;

    //validates using validation data
    double Validate();

    //returns training data 
    const arma::mat& GetTData() const; 

    //returns training labels
    const arma::mat& GetTLabel() const; 

    //returns validation data 
    const arma::mat& GetVData() const; 

    //returns validation labels
    const arma::mat& GetVLabel() const; 

    //overloaded Eval, for single arma rows
    int Eval(arma::Row<double> input);

    //validation on the trianing data
    double Training_Validate();

    //Filters the input data vector to return only data within the margin
    virtual std::shared_ptr<DataVec> MarginFilter(const DataVec& inputDataVec, double margin) const;
protected:
    arma::mat validData_;
    arma::mat validLabel_;

    arma::mat trainingData_;
    arma::mat trainingLabel_;
};

#endif


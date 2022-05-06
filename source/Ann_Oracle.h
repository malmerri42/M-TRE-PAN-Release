#ifndef _ANN_ORACLE_
#define _ANN_ORACLE_

#include <mlpack/core.hpp>
#include <mlpack/methods/ann/layer/layer.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include "Oracle.h"
typedef mlpack::ann::FFN<mlpack::ann::MeanSquaredError<>, mlpack::ann::RandomInitialization> FFANN;
typedef std::shared_ptr<FFANN> FFANNPtr;
//Class that handles creating feed forward backpropagation NNs
class Ann_Oracle : public Oracle
{
public:
    //create and train ANN 
    Ann_Oracle(const char* filename, double valiPercent, size_t epocMax);

    //predict using ANN
    void Eval(DataVec& inputDataVec);

    //Filter out data that is not close to the decision boundry within the margin
    std::shared_ptr<DataVec> MarginFilter(const DataVec& inputDataVec, double margin) const;

private:

    //the created ann pointer
    FFANNPtr ffnPtr_;

};

#endif

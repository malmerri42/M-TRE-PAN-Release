#ifndef _DATA_LIST_H_
#define _DATA_LIST_H_
#include <vector>
//#include <armadillo>
#include <mlpack/core.hpp> //for armadillo
#include <utility>
#include "Constraint.h"
#include "Oracle.h"
//a facade for managing a list of data
//may be implemented with mongodb or armadillo


typedef std::pair< arma::Mat<double>,arma::Col<double> >  MatBiasPair;
typedef std::vector< MatBiasPair > MatBiasPairVec;

class Data_List
{
public:
    //Default constructor
    Data_List();

    //initializing constructor
    //instantiates and initializes an object of type Data_List from an oracle
    Data_List(const char* fileName, bool reLabel, std::shared_ptr<Oracle> oraclePtr);

    //initializing constructor
    //instantiates and initalizes an object of type Data_List from a file
    Data_List(const char* fileName);

    //find entropy of dataList with respect to the current constraints
    double Get_Entropy() const;

    //add a constraint row to the data
    void Add_Constraint( arma::Row<double>  inputRow, double bias, bool eqFlag );

    //add a constraint obj to the data
    //TODO:
    //void Add_Constraint( Constraint constraintObj );

    //overwrite currect constraint with this new one
    void Set_Constraint(const Constraint& inputConstraint);

    //returns a copy of the current constraint
    Constraint Get_Constraint() const;

    //returns a copy of the current constraint
    const Constraint& Get_Constraint_Ref() const { return appliedConstr_;};

    //Adds data as a vector
    void Add_Data( const DataVec &inputData );

    //Return a "copy" of data from the constraints
    //as of C++11, vector can be returned by value as it is on the heap
    DataVec Get_Data() const;
    
    //Returns the maximum and minimum values of the features as a pair of Rows
    //First is the maximum, Second is the minimum
    static RowPair MaxMin(const DataVec &inputData );

    //sample within constraints
    void Sample_On_Demand();

    //driver for testing
    void driver();

    //return refrence to data
    const DataVec& Get_Data_Ref() const;

    //return the majority class in the constrained dataset
    int MajClass() const;

    //return pair of class weights first is for the negative class then the positive class to "even" out the classes
    std::pair<double,double> Get_Class_Weight() const;

    //Get the constraied validation data as a data vector
    DataVec Get_Constrained_V_Data() const;

    //return a refrence to the oracle pointer
    const std::shared_ptr<Oracle> Get_Oracle() const{return oraclePtr_;};

    //copy the current Data_list sans the data
    void No_Data_Copy( Data_List& input) const;

    //return data and label as pair of matrix and column respectivley 
    std::pair<arma::mat, arma::Col<double>> Ret_Data_Arma_Pair() const;

    //set data_ to datavector
    void SetData(const DataVec& input) { data_ = input;};

    //sample data batch
    const std::shared_ptr<DataVec> SampleBatch(size_t amount) const;

    //sample and populate data
    void Sample_And_Populate(size_t amount);

    //return a pruned version of the current constraints
    Constraint Get_Pruned_Constraint() const;

    //return a constraint obj of the inner hcube
    Constraint Get_Inner_Hcube() const;


private:

    //returns the intersections between input row and bias, against innerHcube
    DataVec FindIntersec_(arma::Row<double> inputRow, double bias);

    //return the n choose k combination, where k is dim - 1
    static MatBiasPairVec RetComb_(arma::Mat<double> inMat, arma::Col<double> inBias);

    //helper to update constrained data
    DataVec Get_Data_Constraint_() const;

    //print debug info
    void PrintDebug();

    //Data stored as a vector of armadillo rows, and integer labels
    //content is stored in heap
    DataVec data_;

    //Currently applied constraints
    //Used to create a "view" of data used in training
    Constraint appliedConstr_;

    //Current bounds of inner Hypercube used for sampling on demand
    RowPair innerHcube_;

    //The oracle used to label sampled data
    static std::shared_ptr<Oracle> oraclePtr_;    

    //helper to initialize constraint
    void InitConst_(void);




};

#endif

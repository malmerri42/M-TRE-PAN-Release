#include "Data_List.h"
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <stack>

//defines the cutoff percentage to generate data
#define ENTROPY_CHANGE_PERCENT 1                

//defines the acceptable sample variance for probability
#define SAMPLE_VARIANCE_CUTOFF 0.00001

//can be made a function of data in constraint and area
#define INITIAL_BATCH_SIZE 1000

//Defines the hcube margain
#define HCUBE_MARGIN 0.05


std::shared_ptr<Oracle> Data_List::oraclePtr_(nullptr);

//default constructor
Data_List::Data_List()
{
}

//initializing constructor
////instantiates and initializes and object of type Data_List from a file
////assumption:
////          -:File is preprocessed such that all columns except the last are features
////          -:It has been normalized
////          -:The last column is the label {0 , 1}
//
//Accepts bool to re-label data according to supplied oracle or not
//creates outer and inner box here
Data_List::Data_List(const char* fileName, bool reLabel, std::shared_ptr<Oracle> oraclePtr)
{
    //initialize oraclePtr_
    oraclePtr_ = oraclePtr;

    //load file into matrix
    const arma::mat &trainingData = oraclePtr->GetTData();

    const arma::mat &labelData = oraclePtr->GetTLabel();

    //over all the rows of the input data
    for(size_t i = 0; i < trainingData.n_rows; i++)
    {

        int tempLabel = (labelData(i,0) == 0) ? -1 : 1;
        //label here is the last element of allData for the current row
        Data_List::data_.emplace_back( trainingData.row(i), tempLabel);

    }

    
    //if the relabel flag is set, relabel all the data based on the oracle
    if(reLabel)
    {
        oraclePtr_->Eval(data_);
    }

    //initalize the constraint
    InitConst_();

    return;

}

//helper
//initialize constraint helper
void Data_List::InitConst_(void)
{
    //initialize constraint with an outer h-cube here
    //added to new constraint
    RowPair outMaxMin = MaxMin(data_);  
    
    //initializing innerHcube_ bounds
    innerHcube_ = outMaxMin;


    appliedConstr_.SetMaxMinInit(outMaxMin);

    return;
}

//load data from file
Data_List::Data_List(const char* fileName)
{
    //load data
    arma::mat allData;
    allData.load(fileName);


    //split into data and label
    arma::mat data(allData.cols(0,allData.n_cols - 2));
    arma::Col<double> labels(allData.col(allData.n_cols - 1));

    //over all the rows of the input data
    for(size_t i = 0; i < data.n_rows; i++)
    {

        //label here is the last element of allData for the current row
        data_.emplace_back( data.row(i), labels(i));

    }

    InitConst_();



}

//calculates entropy in dataset currently with respect to current constraints for this instance
double Data_List::Get_Entropy() const
{
    double posExamp = 0, negExamp = 0;

    DataVec constraintData = Get_Data_Ref();

    DataVec::const_iterator dataIterate = constraintData.begin();
    
    //iterate through dataset and count number of positive and negative examples
    for( ; dataIterate != constraintData.end(); ++dataIterate)
    {
        ( (dataIterate->second)== 1 )  ? ( ++posExamp ) : ( ++negExamp);
    }
    
    double posPerc = posExamp/(posExamp + negExamp);
    double negPerc = negExamp/(posExamp + negExamp);
    
   

    std::numeric_limits<double> doubleNumLim;
    //to avoid NaN
    if (posPerc == 0){ return 0;}
    if (negPerc == 0) { return 0;}

    //returns entropy in bits
    return -1 * ( (posPerc * log2(posPerc)) + ( negPerc * log2(negPerc)) );

}

//Adds constraint,
void Data_List::Add_Constraint( arma::Row<double>  inputRow, double bias, bool eqFlag )
{
     

    //check if added constraint "splits", if not throw exception
    //first check how much data is within current constraint
    size_t currentDataAmnt = data_.size(); 

    //add constraint in
    appliedConstr_.Add_Constraint(inputRow,bias,eqFlag);

    //update current data using constraints
    data_ = Get_Data_Constraint_();
    
    //check if list shrunk, if not then this row does not split, throw exception
    size_t newDataAmnt = data_.size();
    if(currentDataAmnt == newDataAmnt)
    {
        throw std::logic_error("cannot split");
    }
    //if there isnt any data it also cannot split
    else if(newDataAmnt == 0)
    {

        throw std::logic_error("cannot split");

    }else{


        //prune constraint list
        //Do not prune here, instead prune only when printing constraints
        //appliedConstr_.PruneConstraint(data_);

        //update innerHcube_ using constrained data_
        //+ a % margin
        const std::pair<arma::Row<double>, arma::Row<double>> MaxMinRows( Data_List::MaxMin(data_));
        arma::Row<double> marginAmnt = arma::abs(MaxMinRows.first - MaxMinRows.second) * HCUBE_MARGIN;
        innerHcube_ = std::pair<arma::Row<double>,arma::Row<double>> (MaxMinRows.first + marginAmnt, MaxMinRows.second - marginAmnt);
    }

}


//overwrite current constraints
void Data_List::Set_Constraint(const Constraint& inputConstraint)
{
    appliedConstr_ = inputConstraint;

    //update current data using constraints
    data_ = Get_Data_Constraint_();
}

//return current constraints
Constraint Data_List::Get_Constraint() const
{
    return appliedConstr_;
}

//blindly add without checking for "conflicts" since it is statistically impossible
void Data_List::Add_Data( const DataVec &inputData )
{


    DataVec::const_iterator dataIterate = inputData.begin();

    
    //iterate through input data and add to existing dataset
    for( ; dataIterate != data_.end(); ++dataIterate)
    {
        data_.push_back(*dataIterate);
    }

}

//get data, simply returns data
DataVec Data_List::Get_Data() const
{
    return data_;
}

//returns const refrence
const DataVec& Data_List::Get_Data_Ref() const
{
    return data_;
}

//used after constraint applied
DataVec Data_List::Get_Data_Constraint_() const
{
    

    DataVec dataHolder;

    DataVec::const_iterator dataIterate = data_.begin();
 

    //iterate through dataset and copy data that is within constraints
    for( ; dataIterate != data_.end(); ++dataIterate)
    {
        if(appliedConstr_.Apply_Constraint(dataIterate->first))
        {
            dataHolder.emplace_back(*dataIterate);  
        }
    }
    
    return dataHolder;
}

//returns row pairs of max and min values of features
RowPair Data_List::MaxMin( const DataVec &inputData )
{
    arma::Row<double> maxRow(inputData[0].first);
    arma::Row<double> minRow(inputData[0].first);

    unsigned int colCounter = 0;
    
    DataVec::const_iterator dataIter = inputData.begin();
    
    for(; dataIter != inputData.end(); dataIter++)
    {
        //check each member of the rows of the input data against current max and min
        for(colCounter = 0; colCounter < dataIter->first.n_cols; colCounter++)
        {
           if((dataIter->first)(colCounter) > maxRow(colCounter))
           {
              maxRow(colCounter) = (dataIter->first)(colCounter);
           }
           else if( (dataIter->first)(colCounter) < minRow(colCounter) )
           {
              minRow(colCounter) = (dataIter->first)(colCounter);
           }
        }
    }

    std::pair< arma::Row<double>, arma::Row<double> > maxMin(maxRow,minRow);

    return maxMin;
}


//Returns unlabeled intersections of the input row against the current innerHcube bounds
//Including intersections that dont involve the row
/*
DataVec Data_List::FindIntersec_(arma::Row<double> inputRow, double bias)
{
    //store in here the unlabeled intersection points
    DataVec intersectList;

    //generate all combinations of n choose k rows of A and B, k being the dimension of the data
    MatBiasPairVec combTemp = RetComb_(inTotalMat, inTotalBias);  

    //generate all intersections
    MatBiasPairVec::const_iterator intersIter = combTemp.begin();
    for(;intersIter != combTemp.end(); intersIter++)
    {
        
        //if there is an intersection, store it in the list with label 0
        arma::Col<double> solHolder;
        bool foundSol = arma::solve(solHolder,intersIter->first,intersIter->second,arma::solve_opts::no_approx);
        if(foundSol)
        {  
            std::pair<arma::Row<double>,int> tempPair(solHolder.t(),0);
            intersectList.push_back(tempPair);
        }
               
    }
    
    return intersectList;
}
*/

//used for testing/debugging
void Data_List::driver()
{
    /*
    arma::Row<double> max = {1, 1};
    arma::Row<double> min = {-1, -1};

    arma::Row<double> lineVal = {-1, 1};
    

    RowPair innerTest(max,min);
    innerHcube_ = innerTest;
    
    arma::Mat<double> test = arma::eye(2,2);

    DataVec intersecs = FindIntersec_(lineVal, 0);
    */
    oraclePtr_->Eval(data_);
    
    Sample_On_Demand();
    

}



//sample within constraints using oracle
void Data_List::Sample_On_Demand()
{
    unsigned long int batchSize = INITIAL_BATCH_SIZE;
    //initial entropy value
    double entropy = Get_Entropy(); 
    bool done = 0;
    
    //sample within inHCubeRange
    //keep going untill last (last batch entropy - first batch entropy)/first batch entropy * 100 < 1
    double totalGenerated = 0; //keeps track of total generated data amount

    //tracking sample X((indicator rand var) expected val (which is the prob, and used for entropy) and variance
    double sampleSum = 0;
    double varianceSum = 0;

    //before generating more data, update the above parameters based on the current data
    DataVec currentData = Get_Data_Ref();
    for(size_t j = 0; j < currentData.size(); j++)
    {

        if(currentData[j].second == 1)
        {
            sampleSum++;
        }
        totalGenerated++;
    }


    while(!done)    
    {
        //generated data in a batch
        std::shared_ptr<DataVec> generatedData;

        generatedData = SampleBatch(batchSize); 
        totalGenerated = totalGenerated + batchSize;

        //label the data
        oraclePtr_->Eval(*generatedData);

        //use generatedData to calculate the sample mean and variance
        double xSampleExpectedVal = 0;
        double xSampleVariance = 0;

        //find the sample sum
        //then the XSampleExpectedVal
        for(size_t k = 0; k < generatedData->size(); k++)
        {
            if((*generatedData)[k].second == 1)
            {
                sampleSum++;
            }
        }

        xSampleExpectedVal = sampleSum/totalGenerated;

        //recalculate the variance sum from the whole data, since the expectedVal changed
        //THIS IS INCORRECT THE VARIANCE OF THE ESTIMATOR FOR INDICATOR RAND VAR IS var[xSampleExpectedVal] = ((xSampleExpectedVal)(1 - xSampleExpectedVal))/totalGenerated
        varianceSum = 0;
        /*
        const DataVec& wholeData = Get_Data_Ref();
        for(size_t k = 0; k < wholeData.size(); k++)
        {
            if(wholeData[k].second == 1)
            {
                varianceSum += std::pow((1 - xSampleExpectedVal), 2);
            }else
            {

                varianceSum += std::pow(( 0 - xSampleExpectedVal), 2);
            }
        }
        //then the contribution of the newly generatedData, then XSampleVariance
        for(size_t k = 0; k < generatedData->size(); k++)
        {
            if((*generatedData)[k].second == 1)
            {
                varianceSum += std::pow((1 - xSampleExpectedVal), 2);
            }else
            {

                varianceSum += std::pow(( 0 - xSampleExpectedVal), 2);
            }
        }
        */

        //NOTE: WE DIVIDE BY TOTAL GENERATED SINCE THIS IS NOW THE VARIANCE OF THE SAMPLE MEAN, NOT THE RandProcess 
        //xSampleVariance = ((1/(totalGenerated - 1)) * varianceSum)/totalGenerated;
        xSampleVariance = ((xSampleExpectedVal)*(1 - xSampleExpectedVal))/totalGenerated;
        //add data to overall data list
        data_.insert(data_.end(), generatedData->begin(), generatedData->end());
        
        //find the change in entropy
        //if it is more that ENTROPY_CHANGE_PERCENT
        //USE SAMPLE VARIANCE on PROBABILITY ESTIMATOR 
        if( xSampleVariance > SAMPLE_VARIANCE_CUTOFF )
        {
            done = 0;
        }else
        {
            done = 1;
        }


    }
}

void Data_List::PrintDebug()
{
    //matrixize the constrained data
    DataVec constData = Get_Data_Ref();

    arma::Mat<double> posDataMat;
    arma::Mat<double> negDataMat;

    DataVec::const_iterator constIter = constData.begin();

    for(; constIter != constData.end(); constIter++)
    {
        //seperate postive and negative data
        if(constIter->second == 1)
        {
            posDataMat = arma::join_vert(posDataMat, constIter->first);
        }else
        {
            negDataMat = arma::join_vert(negDataMat, constIter->first);
        }
    }


    //constraint matrix, intersections and biasVec
    appliedConstr_.PrintDebug();
    posDataMat.save("posDataMat.csv", arma::csv_ascii);
    negDataMat.save("negDataMat.csv", arma::csv_ascii);

}
        
int Data_List::MajClass() const
{
    
    unsigned long int pos = 0;

    unsigned long int neg = 0;

    const DataVec& filteredData = Get_Data_Ref();
     
    DataVec::const_iterator dataIter = filteredData.begin();

    for(; dataIter != filteredData.end(); ++dataIter)
    {
        if(dataIter->second == 1)
        {
            pos++;
        }else
        {
            neg++;
        }
    }
     
    if(pos > neg)
    {
        return 1;
    }else
    {
        return -1;
    }
}

//return pair of class weights first is for the negative class then the positive class to "even" out the classes
std::pair<double,double> Data_List::Get_Class_Weight() const
{
    
    double pos = 0;

    double neg = 0;

    const DataVec& filteredData = Get_Data_Ref();
     
    DataVec::const_iterator dataIter = filteredData.begin();

    for(; dataIter != filteredData.end(); ++dataIter)
    {
        if(dataIter->second == 1)
        {
            pos++;
        }else
        {
            neg++;
        }
    }
     
    //if either is 0, it is nan (as it should be)
    if(pos > neg)
    {
        return std::pair<double,double>(pos/neg, 1);
    }else
    {
        return std::pair<double,double>(1, neg/pos);
    }
}

//return const ref to Constrained validation data from oracle
DataVec Data_List::Get_Constrained_V_Data() const
{
    const arma::mat& vData = oraclePtr_->GetVData();                                      
    const arma::mat& vLabel = oraclePtr_->GetVLabel();                                      
                                                                                         
    DataVec dataHolder;

    //iterate through dataset and copy data that is within constraints
    for(size_t i = 0; i < vData.n_rows; i++)
    {
        if(appliedConstr_.Apply_Constraint(vData.row(i)))
        {
            dataHolder.emplace_back(vData.row(i),vLabel.row(i)(0,0));  
        }
    }
    
    return dataHolder;
}

void Data_List::No_Data_Copy(Data_List& input) const
{
    input.appliedConstr_ = this->appliedConstr_;
    input.innerHcube_ = this->innerHcube_;
}

//return the data as matrix and column for data and label respectivly 
std::pair<arma::mat, arma::Col<double>> Data_List::Ret_Data_Arma_Pair() const
{
    arma::mat outData(0,data_[0].first.n_cols);
    arma::Col<double> outLabel;

    size_t rowCounter = 0;
    DataVec::const_iterator constIter = data_.begin();
    for(; constIter != data_.end(); constIter++)
    {
        outData.insert_rows(rowCounter,constIter->first);
        outLabel.insert_rows(rowCounter,arma::Col<double>() = {(double)constIter->second});
        rowCounter++;
    }
    
    return std::pair<arma::mat, arma::Col<double>>(outData,outLabel);
}

//sample a batch of data using the current constraints
const std::shared_ptr<DataVec> Data_List::SampleBatch(size_t amount) const
{
    std::shared_ptr<DataVec> generatedData = std::make_shared<DataVec>();

    //sample a batch
    size_t i = 0;
    while(i < amount)
    {
        //generate random vector of size dim, between min and max
        arma::Row<double> randData = (arma::randu(1,innerHcube_.first.n_cols) % (innerHcube_.first - innerHcube_.second) + innerHcube_.second);
        
        //if the randData row is within the constraints, increment i, otherwise continue
        if(appliedConstr_.Apply_Constraint(randData))
        {
            i++;

            //initialize as data unit and add to generatedData
            DataUnit dataVal(randData,0);
            generatedData->push_back(dataVal);

        }
    }
    return generatedData;
}

void Data_List::Sample_And_Populate(size_t amount)
{
    data_ = *SampleBatch(amount);
}

Constraint Data_List::Get_Pruned_Constraint() const
{
    //Needs a saftey bound around the data due to problems with "melting" constraints due to hcube shrinkage
    
//  //initialize constraint with an outer h-cube here
//  //added to new constraint
//  RowPair hcubeNoMargin = MaxMin(data_);  

//  //transpose the max and min into columns for the bias
//  //and make diag matrices for the constraints
//  arma::Col<double> max(hcubeNoMargin.first.t());
//  arma::Col<double> min(hcubeNoMargin.second.t());
//  
//  arma::Mat<double> maxCons = arma::eye(max.n_rows,max.n_rows);
//  arma::Mat<double> minCons = arma::eye(min.n_rows,min.n_rows);


//  arma::mat comboMat = arma::join_vert(maxCons, -1 * minCons);
//  arma::Col<double> comboCol = arma::join_vert(max, -1 * min);

    Constraint copyPruned(Get_Constraint());

//  copyPruned.Add_ConstraintMat(maxCons, max, 0);
//  copyPruned.Add_ConstraintMat(minCons, min, 1);

    copyPruned.PruneConstraint(Get_Data_Ref());
    return copyPruned;
}

Constraint Data_List::Get_Inner_Hcube() const
{
    //transpose the max and min into columns for the bias
    //and make diag matrices for the constraints
    arma::Col<double> max(innerHcube_.first.t());
    arma::Col<double> min(innerHcube_.second.t());

    arma::Mat<double> maxCons = arma::eye(max.n_rows,max.n_rows);
    arma::Mat<double> minCons = arma::eye(min.n_rows,min.n_rows);


    arma::mat comboMat = arma::join_vert(maxCons, -1 * minCons);
    arma::Col<double> comboCol = arma::join_vert(max, -1 * min);

    Constraint innerHConstr;
    innerHConstr.SetInitial(comboMat, comboCol);

    return innerHConstr;

}

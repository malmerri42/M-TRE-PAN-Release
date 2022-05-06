#include "Ann_Oracle.h"
#define PROP 1

//Trains the ANN on input data minus the data used for validation
Ann_Oracle::Ann_Oracle(const char* filename, double valiPercent, size_t epocMax):
    Oracle(filename, valiPercent)
{
    
    ffnPtr_ = std::make_shared<FFANN>();

    mlpack::ann::FFN<mlpack::ann::MeanSquaredError<>, mlpack::ann::RandomInitialization> testFFN;

    //input layer, connecting to hidden layer with inputs*2 number of hidden nodes
    testFFN.Add<mlpack::ann::Linear<>>(trainingData_.n_cols,trainingData_.n_cols*PROP);
    //sigmoid activation
    testFFN.Add<mlpack::ann::SigmoidLayer<>>();

    //2nd hidden layer
    testFFN.Add<mlpack::ann::Linear<>>(trainingData_.n_cols*PROP,trainingData_.n_cols*PROP);
    //sigmoid activation
    testFFN.Add<mlpack::ann::SigmoidLayer<>>();

//  //hidden layer
//  testFFN.Add<mlpack::ann::Linear<>>(trainingData_.n_cols*PROP,trainingData_.n_cols*PROP);
//  //sigmoid activation
//  testFFN.Add<mlpack::ann::SigmoidLayer<>>();

//  //hidden layer
//  testFFN.Add<mlpack::ann::Linear<>>(trainingData_.n_cols*PROP,trainingData_.n_cols*PROP);
//  //sigmoid activation
//  testFFN.Add<mlpack::ann::SigmoidLayer<>>();

    //output layer
    testFFN.Add<mlpack::ann::Linear<>>(trainingData_.n_cols*PROP,1);
    //sigmoid activation
    testFFN.Add<mlpack::ann::SigmoidLayer<>>();

     
    //find the accuracy on the validation dataset
    double maxAcc = 0;
    double valAcc = 0;
    double trainAcc = 0;
    bool done = 0;


    ffnPtr_  = std::make_shared< FFANN >(testFFN);
    FFANNPtr maxFFNPtr = std::make_shared< FFANN >(testFFN);
    for(size_t i = 0; i < epocMax; i++)
    {
        ffnPtr_->Train(trainingData_.t(), trainingLabel_.t());
        
        valAcc = Validate();
        trainAcc = Training_Validate();
        std::cout << "Accuracy Training and Validation: ( " << trainAcc << ", " << valAcc << " ) %" << std::endl;
        if( valAcc > maxAcc )
        {
            maxAcc = valAcc;
            //copying seems to be broken in mlpack, instead "copy" parameters
            //this does not work on any models that store memorya (IE non/semi parametric)
            *maxFFNPtr =  *ffnPtr_;
        
        }
    }

    ffnPtr_ = maxFFNPtr;
}

void Ann_Oracle::Eval(DataVec& inputDataVec)
{
    //label all
    DataVec::iterator dataIter = inputDataVec.begin();
    for(; dataIter != inputDataVec.end(); dataIter++)
    {
       arma::mat prediction;
       ffnPtr_->Predict(dataIter->first.t(),prediction); 
       int pVal = arma::round(prediction).at(0,0);

       //if class 0, label to -1
       if(pVal == 0) {dataIter->second = -1;}
       else{ dataIter->second = pVal;}
    }
}

std::shared_ptr<DataVec> Ann_Oracle::MarginFilter(const DataVec& inputDataVec, double margin) const
{
    //Filter all by placing examples within the margin in the data holder
    std::shared_ptr<DataVec> marginDataPtr = std::make_shared<DataVec>();
    DataVec::const_iterator dataIter = inputDataVec.begin();
    for(; dataIter != inputDataVec.end(); dataIter++)
    {
       arma::mat prediction;
       ffnPtr_->Predict(dataIter->first.t(),prediction); 
       double predictVal = prediction(0,0);

       if(0.5 - margin > predictVal > margin + 0.5)
       {
           marginDataPtr->push_back(*dataIter);
       }
    }

    return marginDataPtr;
}

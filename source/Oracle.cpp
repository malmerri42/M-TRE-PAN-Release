#include "Oracle.h"

//constructor using data in a file
Oracle::Oracle(const char* filename, double valiFrac, bool classBalance)
{
    //load training data
    arma::mat allData;
    allData.load(filename);

    //shuffle
    allData = arma::shuffle(allData,0);

    //if enabled, class balance
    if(classBalance)
    {
        //sum of all elements to get the number of exceeding labels
        arma::mat rowSum = arma::sum(allData,0);

        long long posAmnt = rowSum(0,allData.n_cols - 1);
        long long negAmnt = allData.n_rows - posAmnt;

        long long diff = 0;
        int toFilter = 0;
        
        // depending on which class is larger, filter it till they are equal 
        if(posAmnt > negAmnt)
        {
            diff = posAmnt - negAmnt;

            toFilter = 1;

        }else if(posAmnt < negAmnt)
        {
            diff = negAmnt - posAmnt;

            toFilter = 0;

        }


        arma::mat tempHolder(allData.n_rows - diff,allData.n_cols, arma::fill::zeros);
        size_t index = 0;
        size_t newIndex = 0;
        for(index = 0; index < allData.n_rows; index++)
        {
            //didnt hit filtered value case
            if(allData(index,allData.n_cols - 1) != toFilter)
            {
                tempHolder.row(newIndex) = allData.row(index);
                newIndex++;
            }else
            {
                //check if we are "done" filtering
                //if we are (diff = 0), then just copy anyway
                if(diff > 0)
                {
                    diff--;
                }else
                {
                    tempHolder.row(newIndex) = allData.row(index);
                    newIndex++;
                }
            }

        }

        allData = tempHolder;

        //reshuffle data
        allData = arma::shuffle(allData,0);


    }

    //seperate into training data and labels
    //same goes for validation
    arma::mat data(allData.cols(0,allData.n_cols - 2));
    arma::mat labels(allData.col(allData.n_cols - 1));

    validData_ = data.rows(0,floor((data.n_rows - 1)* valiFrac));
    validLabel_ = labels.rows(0,floor((labels.n_rows - 1)* valiFrac));

    trainingData_ = data.rows(floor((data.n_rows - 1)* valiFrac + 1),data.n_rows - 1);
    trainingLabel_ = labels.rows(floor((labels.n_rows - 1)* valiFrac + 1),labels.n_rows - 1);

    


}


//convert Eval class -1 to class 0
double Oracle::Validate()
{


    arma::Col<double> predictions(validLabel_.n_rows);

    for(unsigned long i = 0; i < validData_.n_rows; i++)
    {
        //correctly assign "negative" class as zero from eval
        int tempPredict = Oracle::Eval(validData_.row(i));
        if(tempPredict == -1)
        {
            predictions(i) = 0;

        }else
        {
            predictions(i) = tempPredict;
        }
    }

    //print accuracy
    //First, get amnt of nonzero elements after subtraction, since each zero represents a true prediction
    arma::Col<double> nonzeros = (arma::nonzeros(predictions - validLabel_));
    double correctPredictions = predictions.n_rows - nonzeros.n_rows;
    return (correctPredictions/predictions.n_rows);

}

//accuracy on the training dataset
double Oracle::Training_Validate()
{


    arma::Col<double> predictions(trainingLabel_.n_rows);

    for(unsigned long i = 0; i < trainingData_.n_rows; i++)
    {
        //correctly assign "negative" class as zero from eval
        int tempPredict = Oracle::Eval(trainingData_.row(i));
        if(tempPredict == -1)
        {
            predictions(i) = 0;

        }else
        {
            predictions(i) = tempPredict;
        }
    }

    //print accuracy
    //First, get amnt of nonzero elements after subtraction, since each zero represents a true prediction
    arma::Col<double> nonzeros = (arma::nonzeros(predictions - trainingLabel_));
    double correctPredictions = predictions.n_rows - nonzeros.n_rows;
    return (correctPredictions/predictions.n_rows);

}

const arma::mat& Oracle::GetVData() const
{
    return validData_;
}

const arma::mat& Oracle::GetVLabel() const
{
    return validLabel_;
}

const arma::mat& Oracle::GetTData() const
{
    return trainingData_;
}

const arma::mat& Oracle::GetTLabel() const
{
    return trainingLabel_;
}
int Oracle::Eval(arma::Row<double> input)
{
    //create single element vector
    //hacky solution
    DataVec singleVec = { DataUnit(input,0) };

    Eval(singleVec);

    return singleVec[0].second;

}

std::shared_ptr<DataVec> Oracle::MarginFilter(const DataVec& inputDataVec, double margin) const
{
    //Filter all by placing examples within the margin in the data holder
    std::shared_ptr<DataVec> marginDataPtr = std::make_shared<DataVec>();
    DataVec::const_iterator dataIter = inputDataVec.begin();
    for(; dataIter != inputDataVec.end(); dataIter++)
    {

        marginDataPtr->push_back(*dataIter);
    }

    return marginDataPtr;
}

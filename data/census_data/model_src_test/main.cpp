#include "../../../source/mlpack/src/mlpack/core.hpp"

#include <mlpack/methods/ann/layer/layer.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>

#define EPOCHS 5
template class arma::Mat<double>;
template class arma::Col<double>;
template class arma::Row<std::size_t>;
template class arma::Col<size_t>;
template class arma::SpMat<double>;
int main()
{
    arma::mat trainingData;
    trainingData.load("../model_src_test/test_census_data_no_cat.csv");
    arma::mat trainingLabel;
    trainingLabel.load("../model_src_test/test_labels_no_cat.csv");

    arma::mat validationData;
    validationData.load("../model_src_test/census_data_no_cat.csv");
    arma::mat validationLabel;
    validationLabel.load("../model_src_test/labels_no_cat.csv");

    mlpack::ann::FFN<mlpack::ann::MeanSquaredError<>, mlpack::ann::RandomInitialization> testFFN;

    //input layer, connecting to hidden layer with inputs*2 number of hidden nodes
    testFFN.Add<mlpack::ann::Linear<>>(trainingData.n_cols,trainingData.n_cols*2);
    //sigmoid activation
    testFFN.Add<mlpack::ann::SigmoidLayer<>>();

    //hidden layer
    testFFN.Add<mlpack::ann::Linear<>>(trainingData.n_cols*2,trainingData.n_cols*2);
    //sigmoid activation
    testFFN.Add<mlpack::ann::SigmoidLayer<>>();

    //output layer
    testFFN.Add<mlpack::ann::Linear<>>(trainingData.n_cols*2,1);
    //sigmoid activation
    testFFN.Add<mlpack::ann::SigmoidLayer<>>();

    for(int i; i < EPOCHS; i++)
    {
        std::ofstream reportFile;
        //std::ostringstream name;
        //name << "FFN_report" << i << ".txt";
        reportFile.open("FFN_report.txt", std::ios_base::app); 

        testFFN.Train(trainingData.t(), trainingLabel.t(), ens::Report(1,reportFile));

        reportFile.close();
    }

    arma::mat assignments;

    testFFN.Predict(validationData.t(),assignments);

    //std::cout<<"Predictions     : "<<assignments<<std::endl;
    //std::cout<<"CorrectLabels   : "<<validationLabel.t()<<std::endl;

    arma::mat combMat = validationLabel.t() - arma::round(assignments);

    double trueSampleAmt = 0;
    double allSampleAmt = 0;

    for(int i = 0; i < combMat.n_cols; i++)
    {
        allSampleAmt ++;
        if(combMat(0,i) == 0)
        {
            trueSampleAmt ++;
        }
    }

    std::cout << "%" << trueSampleAmt/allSampleAmt * 100 << std::endl;

    return 1;
}

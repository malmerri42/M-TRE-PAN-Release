#include "Deci_Tree_Node.h"
#include "MyStats.h"
#include <mlpack/core.hpp>
#include <mlpack/methods/linear_svm/linear_svm.hpp>

void Deci_Tree_Node::Train_()
{
    //prepare data for use with mlpack::svm
    DataVec constraintData = nodeData_.Get_Data();

    //set up training data (bad way to get dimension btw)
    arma::Mat<double> trainingData(constraintData.size(), constraintData[0].first.n_cols);
    arma::Row<size_t> labels(constraintData.size());

    unsigned long i = 0;
    DataVec::const_iterator dataIter = constraintData.begin();
    for(; dataIter != constraintData.end(); dataIter++)
    {
        trainingData.row(i) = dataIter->first;

        //for explicitness we do the extra == operation
        //this means data that was not correctly labeled leaked into the dataset
        //more efficently done with a hash/map
        if(dataIter->second == 0){throw std::runtime_error("data leak, incorrect label"); }
        else if(dataIter->second == -1){ labels(0,i) = 0; }
        else { labels(0,i) = 1;}

        i++;
    }




    //Normalize the data
    std::pair<arma::mat, arma::mat> normDataPair = MyStats::Norm(trainingData, -1, 1);

    arma::mat d_training = normDataPair.first.t();

    //mlpack::tree::DecisionTree<> myDesTree( d_training, labels, 2,  10, 1e-7, 2);

    //try an svm on a "squashed" dataset
    
    //make vector of svms for every dimension 
    //train and find the one with the MINIMUM objective val (the loss)
    double minLoss = std::numeric_limits<double>::max();
    long long minID = -1;
    std::vector<mlpack::svm::LinearSVM<>> svmVect;
    for(long long i = 0; i < d_training.n_rows; i++)
    {
        ens::L_BFGS optimizer(10,0,1e-4,0.9,1e-6,1e-15,100);

        svmVect.emplace_back(2, 0.0001, 1, 1);
        double loss = svmVect[i].Train(d_training.row(i), labels, 2, optimizer);
        
        //update min
        if( loss < minLoss)
        {
            minLoss = loss;
            minID = i;
        }

    }
    if(minID == -1){ throw std::string("min was not found, svm is broken"); }

    //parameters of the tree consist of the split on the dimension 
    //ex: c1*x + c2*y < b
    //if our split point is on x at 0.3
    //then the parameters are c1 = 1; c2 = 0; b = 0.3
    arma::Row<double> d_parameters(d_training.n_rows + 1, arma::fill::zeros);
    //the "first" param
    d_parameters(minID) = svmVect[minID].Parameters()(0,0);
    //bias
    d_parameters(d_parameters.n_elem - 1) = svmVect[minID].Parameters()(1,0);

    //Denormalize the parameters
    arma::Row<double> denormParam = MyStats::DeNorm(d_parameters, normDataPair.second, -1, 1);

    //extract the information of the seperator from the SVM
    //making sure to check for NaNs
    if(denormParam.has_nan()) { throw std::runtime_error("parameters have NaN"); }


    //Multiply the bias terms by negative -1, to match the interpretation of bias in this program
    //params.row(params.n_rows - 1) * -1;

    //this only grabs one of the hyperplanes
    //to get optimal hyperplane
    RawConstraint svmParameters( denormParam.cols(0,denormParam.n_cols - 2), -1 * denormParam(denormParam.n_cols - 1));

    //make a copy of the constraint for the tree node, to use when evaluating input
    //Fitting in this constraint, make you go to the left child, else the right one
    treeConstraint_.Add_Constraint(svmParameters.first, svmParameters.second, 1);

    //add constraint to dataList copy for left child and for right child
    Data_List leftData = nodeData_;
    Data_List rightData = nodeData_;


    leftData.Add_Constraint( svmParameters.first, svmParameters.second, 1);
    rightData.Add_Constraint( svmParameters.first, svmParameters.second, 0);

    //increment depth
    //supply self as parent ptr
    //build left and right leaf
    leftChild_ = std::make_shared<Leaf_Node>(leftData, depth_ + 1, this, -1);
    rightChild_ = std::make_shared<Leaf_Node>(rightData, depth_ + 1, this, 1);
}

#include "Tree_Node.h"
#include <memory>
#include <mlpack/core.hpp>
#include <mlpack/methods/linear_svm/linear_svm.hpp>
#include "MyStats.h"

#define MAX_LINESEARCH_ITER 100

Tree_Node::Tree_Node(Data_List inputData, unsigned int currentDepth, Tree_Node* const parentPtr, int label):
    Base_Tree_Node(inputData, currentDepth, label),
    treeConstraint_(inputData.Get_Constraint()),
    parent_(parentPtr)
{

    //TODO: DELETE THIS DEBUG CHECK
    //double debugEntropy = inputData.Get_Entropy();
    //std::cout << "tree node created at depth: "  << currentDepth << std::endl;

    //call Train_() to set the correct values for seperator
    //Places in Data_List the correct new constraint, then makes a copy of it for the TreeNode

}

//returns the raw hyperplane from a linear SVM trained on the constrained data of this node
void Tree_Node::Train_()
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

    std::numeric_limits<double> doubleNumLim;
    //make the optimizer to specify settings
    ens::L_BFGS optimizer(10,0,1e-4,0.9,1e-6,1e-15,MAX_LINESEARCH_ITER); 

    //Normalize the data to help the SVM
    //btw you can use ARMA for this, treat each column as a vector, and make it a unit vector
    std::pair<arma::mat, arma::mat> normDataPair = MyStats::Norm(trainingData, -1, 1);

    //DEBUG REPORTER
    //std::ofstream reportFile;
    //reportFile.open("report_tree.txt");
    mlpack::svm::LinearSVM<> myLinearSVM( normDataPair.first.t(), labels, 2, 0.0001, 1, 1, optimizer); 
    //reportFile.close(); 

    //Denormalize the parameters
    arma::Row<double> denormParam = MyStats::DeNorm(myLinearSVM.Parameters().col(0).t(), normDataPair.second, -1, 1);
    
    //extract the information of the seperator from the SVM
    //making sure to check for NaNs 
    if(denormParam.has_nan()) { 

        //DEBUG REPORTER
      //std::ofstream reportFile;
      //reportFile.open("report_svm_tree.txt");
      //mlpack::svm::LinearSVM<> myLinearSVM( normDataPair.first.t(), labels, 2, 0.0001, 1, 1, optimizer, ens::Report(1,reportFile)); 
      //reportFile.close(); 
        throw std::runtime_error("parameters have NaN"); 
    }


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

int Tree_Node::Evaluate(const arma::Row<double> &inputExample) const
{
    //if the provided example is not within in the constraints
    if(!nodeData_.Get_Constraint_Ref().Apply_Constraint(inputExample)){throw;} 

    //0 is the "nan" of our labeling
    int label = 0;

    if(treeConstraint_.Apply_Constraint(inputExample))
    {
        label = leftChild_->Evaluate(inputExample);
    }else
    {
        label = rightChild_->Evaluate(inputExample);
    }
    
    if(label == 0)
    {
        throw;
    }

    return label;
}

int Tree_Node::Evaluate(const arma::Row<double> &inputExample, size_t depthLim) const
{
    //if the provided example is not within in the constraints
    if(!nodeData_.Get_Constraint_Ref().Apply_Constraint(inputExample)){throw;} 
    if(depth_ < depthLim)
    {
        //0 is the "nan" of our labeling
        int label = 0;

        if(treeConstraint_.Apply_Constraint(inputExample))
        {
            label = leftChild_->Evaluate(inputExample, depthLim);
        }else
        {
            label = rightChild_->Evaluate(inputExample, depthLim);
        }
        
        if(label == 0)
        {
            throw;
        }

        return label;
    }else
    {
        return label_;
    }
}

Base_Tree_Node* const Tree_Node::Get_Parent()
{
    return parent_;
}

//returns the child pair
ChildPair Tree_Node::Get_Children()
{
    return ChildPair(leftChild_,rightChild_);
}



size_t Tree_Node::CountLeaf()
{
    return leftChild_->CountLeaf() + rightChild_->CountLeaf();
}

size_t Tree_Node::CountLeaf(size_t depth)
{
    if(depth_ < depth)
    {
        return leftChild_->CountLeaf(depth) + rightChild_->CountLeaf(depth);
    }else
    {
        return 1;
    }
}

Base_Tree_Node::ConstrVecPtr Tree_Node::PrintConstr(size_t depthLim)
{
    //if given depth limit is reached
    if(depth_ >= depthLim)
    {
        std::vector<std::tuple<Constraint, int, const Base_Tree_Node*>> tempVec;
        //tree nodes are always splitting candidates so the label is uncertain
        tempVec.emplace(tempVec.end(), nodeData_.Get_Constraint(), 0, this);
        ConstrVecPtr tempPtr = std::make_shared<std::vector<std::tuple<Constraint,int, const Base_Tree_Node*>>>(tempVec);
        return tempPtr;

    }else//send call to left and right children, then combine the vectors
    {
        ConstrVecPtr leftPtr = leftChild_->PrintConstr(depthLim);
        ConstrVecPtr rightPtr = rightChild_->PrintConstr(depthLim);

        //combine vectors
        //then return
        leftPtr->insert(leftPtr->end(), rightPtr->begin(), rightPtr->end());
        return leftPtr;

    }
}

//Calculate class balanced model parity pair
//numerator first member then denom
double Tree_Node::BalancedModelParity(size_t depthLim, double percent)
{
    //passes the call down to child nodes
    if(depth_ < depthLim) 
    {
        double leftResult = leftChild_->BalancedModelParity(depthLim, percent);
        double rightResult = rightChild_->BalancedModelParity(depthLim, percent);
        return (leftResult + rightResult)/2;
    }
    //acts as a leaf
    else
    {
        //based on if the parent is null or another node, get the weights they used during training
        double negWeight = 1;
        double posWeight = 1;

        if(parent_ != NULL) 
        {
            std::pair<double,double> weightPair(parent_->GetData_List().Get_Class_Weight());
            negWeight = weightPair.first;
            posWeight = weightPair.second; 
        }

        return Node_Accuracy(negWeight, posWeight, percent);
    }
}

Data_List Tree_Node::RetSampledDataNode(double percent) const
{
    Data_List sampledData;
    nodeData_.No_Data_Copy(sampledData);
    sampledData.SetData( *(this->SampledDataVec(percent)) );

    return sampledData;
}

const std::shared_ptr<DataVec> Tree_Node::SampledDataVec(double percent) const
{
    std::shared_ptr<DataVec> leftSampledData = leftChild_->SampledDataVec(percent);
    std::shared_ptr<DataVec> rightSampledData = rightChild_->SampledDataVec(percent);
    
    leftSampledData->insert(leftSampledData->end(), rightSampledData->begin(), rightSampledData->end());
    
    return leftSampledData;
}

double Tree_Node::Average_Tree_Accuracy(size_t depthLim, double percent) 
{
    //pass call down and recombine
    //can restructure this kind of code with a visitor pattern
    if(depth_ < depthLim)
    {
        double leftAvg = leftChild_->Average_Tree_Accuracy(depthLim, percent);
        double rightAvg = rightChild_->Average_Tree_Accuracy(depthLim, percent);
        return (leftAvg + rightAvg)/2;
    }else
    {
        return Node_Accuracy(1, 1, percent);
    }
}
    
int Tree_Node::Certain_Evaluate(const arma::Row<double> &inputExample, size_t depthLim) const
{
    //if the provided example is not within in the constraints
    if(!nodeData_.Get_Constraint_Ref().Apply_Constraint(inputExample)){throw;} 

    if(depth_ < depthLim)
    {
        int label = 0;
        if(treeConstraint_.Apply_Constraint(inputExample))
        {
            label = leftChild_->Certain_Evaluate(inputExample, depthLim);
        }else
        {
            label = rightChild_->Certain_Evaluate(inputExample, depthLim);
        }
    
        return label;
    }else{

        return 0;
    }
}


size_t Tree_Node::CountCertainLeafs() const
{
    return leftChild_->CountCertainLeafs() + rightChild_->CountCertainLeafs();
}

size_t Tree_Node::CountCertainLeafs(size_t depth) const
{
    if(depth_ < depth)
    {
        return leftChild_->CountCertainLeafs(depth) + rightChild_->CountCertainLeafs(depth);
    }else
    {
        return 0;
    }
}

void Tree_Node::AcceptVisitor (arma::Row<double> dataPoint, Base_Visitor* const visitor)
{
    //Pass self to the visitor, and then have correct child accept
    visitor->VisitNode(this);

    if(treeConstraint_.Apply_Constraint(dataPoint))
    {
        leftChild_->AcceptVisitor(dataPoint,visitor);
    }else
    {
        rightChild_->AcceptVisitor(dataPoint,visitor);
    }
}

#include "Leaf_Node.h"
#include "Tree_Node.h"
#include <utility>
#include <vector>

//Initializing Constructor,
//creates leaf, depending on caller to assign correct values, (caller handles all correct assignment)
Leaf_Node::Leaf_Node(const Data_List &nodeData, unsigned int depth, Tree_Node* const parent, int label):
    Base_Tree_Node(nodeData, depth, label),
    parent_(parent),
    //always assume you can be split, untill you are checked by the tree builder
    splittingCandidate_(1)
{
    //generates more data if needed
   // std::cout << "Leaf node created at depth: " << depth << std::endl;
    nodeData_.Sample_On_Demand();
}

//returns majority class in leaf
int Leaf_Node::Evaluate(const arma::Row<double> &inputExample) const
{
    //if the provided example is not within in the constraints
    if(!nodeData_.Get_Constraint_Ref().Apply_Constraint(inputExample)){throw;} 

    return Evaluate();
}

int Leaf_Node::Evaluate(const arma::Row<double> &inputExample, size_t depthLim) const
{
    
    //if the provided example is not within in the constraints
    if(!nodeData_.Get_Constraint_Ref().Apply_Constraint(inputExample)){throw;} 
    return Evaluate();
}

int Leaf_Node::Evaluate() const
{

    return label_;
}

Base_Tree_Node* const Leaf_Node::Get_Parent()
{
    return parent_;
}

ChildPair Leaf_Node::Get_Children()
{
    return ChildPair(NULL,NULL);
}


bool Leaf_Node::Share_Any_Ref(IntersectionList::const_iterator firstPoint, IntersectionList::const_iterator secondPoint)
{
    bool anyMatch = 0;
    
    for(int i = 0; i < firstPoint->second.size(); i++)
    {
        for(int j = 0; j < secondPoint->second.size(); j++)
        {
            if(firstPoint->second[i] == secondPoint->second[j])
            {
                anyMatch = 1;
            }
        }
    }

    return anyMatch;
}

size_t Leaf_Node::CountLeaf()
{
    return 1;
}

size_t Leaf_Node::CountLeaf(size_t depth)
{
    return 1;
}

Base_Tree_Node::ConstrVecPtr Leaf_Node::PrintConstr(size_t depthLim)
{
    std::vector<std::tuple<Constraint, int, const Base_Tree_Node*>> tempVec;

    if(!splittingCandidate_)
    {
        tempVec.emplace(tempVec.end(),nodeData_.Get_Constraint(),label_, this);
    }else
    {
        tempVec.emplace(tempVec.end(),nodeData_.Get_Constraint(),0, this);
    }

    ConstrVecPtr tempPtr = std::make_shared< std::vector<std::tuple<Constraint, int, const Base_Tree_Node*>> >(tempVec);
    return tempPtr;
}

//find class balanced model parity
double Leaf_Node::BalancedModelParity(size_t depthLim, double percent) 
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

//Returns the nodeData Data_List of all nodes sampled and merged
Data_List Leaf_Node::RetSampledDataNode(double percent) const
{
    Data_List sampledData;
    nodeData_.No_Data_Copy(sampledData);
    sampledData.SetData(*SampledDataVec(percent));

    return sampledData;
}

//Returns the node sampled data vector pointer, amount sampled is percentage of total data this node used for 
//training.
const std::shared_ptr<DataVec> Leaf_Node::SampledDataVec(double percent) const
{
    size_t trainingDataAmnt = nodeData_.Get_Data_Ref().size();
    //testing
    std::shared_ptr<DataVec> vecPtr = nodeData_.SampleBatch(trainingDataAmnt * percent);
    return vecPtr;
}

double Leaf_Node::Average_Tree_Accuracy(size_t depthLim, double percent)
{
    return Node_Accuracy(1,1,percent);
}

int Leaf_Node::Certain_Evaluate(const arma::Row<double> &inputExample, size_t depthLim) const
{
    //if the provided example is not within in the constraints
    if(!nodeData_.Get_Constraint_Ref().Apply_Constraint(inputExample)){throw;} 

    //if this leaf is a candidate for splitting
    if(splittingCandidate_)
    {
        return 0;
    }else
    {
        return Evaluate();
    }
}

size_t Leaf_Node::CountCertainLeafs() const
{
    //if you are a splitting candidate a 0 is returned, else 1 is returned
    return !splittingCandidate_;
}

size_t Leaf_Node::CountCertainLeafs(size_t depthLim) const
{
    //if you are a splitting candidate a 0 is returned, else 1 is returned
    return !splittingCandidate_;
}

void Leaf_Node::AcceptVisitor (arma::Row<double> dataPoint, Base_Visitor* const visitor)
{
    visitor->VisitNode(this);
}

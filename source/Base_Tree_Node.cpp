#include "Base_Tree_Node.h"


//constructor
//creates base node containing inputData, and the current depth from caller
//Parent handles adding new constraints to inputData (a tree node when constructing leaves adds the trained constraint to the Data_List object before passing it)
Base_Tree_Node::Base_Tree_Node(const Data_List& inputData, unsigned int currentDepth, int label):
    nodeData_(inputData),
    depth_(currentDepth),
    label_(label)
{
    inputData.No_Data_Copy(validNodeData_);
}

//Get entropy when this was generated
//since data may grow and change the entropy
double Base_Tree_Node::Get_Entropy() const
{
    return nodeData_.Get_Entropy();
}


//comparison operator between entropy
bool Base_Tree_Node::operator< (const Base_Tree_Node& RHS)
{
    if(this -> Get_Entropy() < RHS.Get_Entropy())
    {
        return 1;
    }else
    {
        return 0;
    }
}

Constraint Base_Tree_Node::GetConstr()
{
    return nodeData_.Get_Constraint();
}

//get model parity "as a leaf" helper func
std::pair<double,double> Base_Tree_Node::As_A_Leaf_Parity_Pair_(double negWeight, double posWeight, double percent) 
{
    
    //TESTING
    //Data_List newValidNodeData;
    //nodeData_.No_Data_Copy(newValidNodeData);
    //newValidNodeData.Sample_On_Demand();

    //const DataVec valDataset = nodeData_.Get_Constrained_V_Data(); 
    const DataVec& valDataset = Sample_DataVec_As_Leaf(percent);				

    //to count evaluated predicitons                                                     
    double correctPrediction = 0;                                                        
    double totalPrediction = 0;                                                          
											 
    DataVec::const_iterator constIter = valDataset.begin();

    //for all the validation data compare against model predictions                      
    int leafNodeGuess = label_;
    for(; constIter != valDataset.end(); constIter++)                                              
    {                                                                                    
	//DONT account for eval returning -1 on "negative" predictions                   
	//since tree will return -1 on negative predictions                              
        int oracleResult = nodeData_.Get_Oracle()->Eval(constIter->first);

	if(oracleResult  == leafNodeGuess  )
	{                                                                                
	    if (oracleResult == -1)
            {
                correctPrediction = correctPrediction + negWeight;
            }else if (oracleResult == 1)
            {
                correctPrediction = correctPrediction + posWeight;
            }else
            {
                //checks if something goes really wrong
                throw;
            }
											 
	}                                                                                
											 
        if (oracleResult == -1)
        {
            totalPrediction = totalPrediction + negWeight;
        }else if (oracleResult == 1)
        {
            totalPrediction = totalPrediction + posWeight;
        }else
        {
        //checks if something goes really wrong
            throw;
        }                                                                                    
    }

    return std::pair<double,double>(correctPrediction, totalPrediction);
}

//return DataVec of node sampled as leaf
const DataVec& Base_Tree_Node::Sample_DataVec_As_Leaf(double percent) 
{
    if(std::floor(nodeData_.Get_Data_Ref().size() * percent) == validNodeData_.Get_Data_Ref().size())
    {
        return validNodeData_.Get_Data_Ref();
    }else
    {
        validNodeData_.Sample_And_Populate(std::floor(nodeData_.Get_Data_Ref().size() * percent));
        return validNodeData_.Get_Data_Ref();
    }
}

double Base_Tree_Node::Node_Accuracy(double negWeight, double posWeight, double percent)
{
    std::pair<double,double> parityFrac = As_A_Leaf_Parity_Pair_(negWeight, posWeight, percent); 
    
    return (parityFrac.first/parityFrac.second);
}

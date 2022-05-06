#include "Branch_Builder.h"

Branch_Builder::Branch_Builder(arma::Row<double> expandPoint, const char* file, std::shared_ptr<Oracle> inputOracle, bool relabelFlag, unsigned long maxdepth, double entropyCutoff):
    Tree_Builder(file, inputOracle, relabelFlag, maxdepth, entropyCutoff),
    expandPoint_(expandPoint)
{
}
//push leaf nodes
void Branch_Builder::PushLeafNodes_(std::stack<LeafPtr>& leafPrioQueue, LeafPtrPair& ptrPair)
{
    if(ptrPair.first->GetConstr().Apply_Constraint(expandPoint_))
    {
        leafPrioQueue.push(ptrPair.first);
    }else
    {
        leafPrioQueue.push(ptrPair.second);
    }
}

//push root leaf nodes
void Branch_Builder::PushRootLeafNodes_(std::stack<LeafPtr>& leafPrioQueue, BaseNodePtr& rootPtr)
{
        //Uses dynamic casting, to push root onto prio queue
    if( (std::dynamic_pointer_cast<Tree_Node>(treeRoot_)->leftChild_)->GetConstr().Apply_Constraint(expandPoint_) )
    {
        leafPrioQueue.push(std::dynamic_pointer_cast<Leaf_Node>(std::dynamic_pointer_cast<Tree_Node>(treeRoot_)->leftChild_));
    }else{
        leafPrioQueue.push(std::dynamic_pointer_cast<Leaf_Node>(std::dynamic_pointer_cast<Tree_Node>(treeRoot_)->rightChild_));
    }
        
}

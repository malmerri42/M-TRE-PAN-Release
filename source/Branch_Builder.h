#ifndef _BRANCH_BUILDER_H_
#define _BRANCH_BUILDER_H_
#include "Tree_Builder.h"

class Branch_Builder : public Tree_Builder
{
    public:
    Branch_Builder(arma::Row<double> expandPoint, const char* file, std::shared_ptr<Oracle> inputOracle, bool relabelFlag = 0, unsigned long maxdepth = 0, double entropyCutoff = 0); 

    protected:
    //push leaf nodes
    virtual void PushLeafNodes_(std::stack<LeafPtr>& leafPrioQueue, LeafPtrPair& ptrPair);
    
    //push root leaf nodes
    virtual void PushRootLeafNodes_(std::stack<LeafPtr>& leafPrioQueue, BaseNodePtr& rootPtr);

    //point to expand on
    const arma::Row<double> expandPoint_;
};


#endif

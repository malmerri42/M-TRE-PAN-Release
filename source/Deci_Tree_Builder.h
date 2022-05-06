#ifndef _DECI_TREE_BUILDER_H_
#define _DECI_TREE_BUILDER_H_
#include "Tree_Builder.h"


class Deci_Tree_Builder : public Tree_Builder
{
public:
    //init constructor 
    Deci_Tree_Builder(const char* file, std::shared_ptr<Oracle> inputOracle, bool relabelFlag = 0, unsigned long maxdepth = 0, double entropyCutoff = 0):
            Tree_Builder(file, inputOracle, relabelFlag, maxdepth, entropyCutoff) { }

    //void Get_All(bool classBalance = 1);

    //void PrintLeafConstr(size_t depth);

    std::string GetTitle() const;
protected:
    //overwrite funcs
    LeafPtrPair Expand_Leaf_(std::shared_ptr<Leaf_Node> inputLeafPtr, std::mutex& cv_m);

    void MakeTree_(const Data_List& inputDataList);
};

#endif

#ifndef _DECI_TREE_NODE_H_
#define _DECI_TREE_NODE_H_

#include "Tree_Node.h"
#include "Deci_Tree_Builder.h"

class Deci_Tree_Builder;

class Deci_Tree_Node : public Tree_Node
{
friend class Deci_Tree_Builder;
public:
    //constructor for Tree_Node 
    Deci_Tree_Node(Data_List inputData, unsigned int currentDepth, Tree_Node* const parentPtr, int label):
        Tree_Node(inputData, currentDepth, parentPtr, label) { }


    void  Train_();
    
};



#endif

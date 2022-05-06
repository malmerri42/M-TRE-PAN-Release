#ifndef _TREE_NODE_H_
#define _TREE_NODE_H_

#include "Constraint.h"
#include <memory>
#include <tuple>
#include "Leaf_Node.h"
#include "Base_Tree_Node.h"

typedef std::pair< arma::Row<double>, double> RawConstraint;

class Tree_Builder;
class Deci_Tree_Builder;
class Branch_Builder;

class Tree_Node : public Base_Tree_Node
{

friend class Tree_Builder;
friend class Deci_Tree_Builder;
friend class Branch_Builder;
public:
    //constructor
    Tree_Node(Data_List inputData, unsigned int currentDepth, Tree_Node* const parentPtr, int label_);

    //evaluate input data example and return class label
    //basically call this for the correct child (recursive)
    int Evaluate (const arma::Row<double> &inputExample) const;

    //overload for depth limit
    int Evaluate (const arma::Row<double> &inputExample, size_t depthLim) const;

    //return the parent pointer
    Base_Tree_Node* const Get_Parent();

    //return the children pair
    ChildPair Get_Children();

    //Trains leaf node, prior to assigning new parent constraints to leaf nodes as current parent constraints union current constraints
    virtual void Train_();

    //passes call to children 
    size_t CountLeaf();

    size_t CountLeaf(size_t depth);

    //passes call to children UNLESS depth requirement is met other wise it acts as leaf
    //then it combines returns from each child and returns to parent
    ConstrVecPtr PrintConstr(size_t depth);
 
    //passes call to children UNLESS depth requirement is met otherwise it acts as leaf
    //then it combines returns from each child and returns to parent
    double BalancedModelParity(size_t depth, double percent);

    //sample children nodes and return in Data_List with same constraints as this node
    Data_List RetSampledDataNode(double percent) const;

    //Returns the sampled node data vector merged of children
    const std::shared_ptr<DataVec> SampledDataVec(double percent) const;

    //Returns the average accuracy of the tree across it's nodes
    double Average_Tree_Accuracy(size_t depthLim, double percent);

    //When depth_ < depthLim it passes the call down, otherwise returns 0
    int Certain_Evaluate(const arma::Row<double> &inputExample, size_t depthLim) const;
    
    //same procedure as count leaf, only since a tree node is automatically a candidate for splitting, it returns 0   
    size_t CountCertainLeafs() const;
    size_t CountCertainLeafs(size_t depth) const;

    //Accept visitor and send it to correct child
    void AcceptVisitor (arma::Row<double> dataPoint, Base_Visitor* const visitor);
protected:
    //constraint used to decide between traversing left and right
    //acquired from "training"
    //a copy of the constraint placed into Data_List
    Constraint treeConstraint_;

    //right child tree node
    std::shared_ptr<Base_Tree_Node> rightChild_;

    //left child tree node
    std::shared_ptr<Base_Tree_Node> leftChild_;

    //parent of this node
    Tree_Node* const parent_; 

};
#endif

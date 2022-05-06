#ifndef _LEAF_TREE_NODE_H_
#define _LEAF_TREE_NODE_H_

#include "Base_Tree_Node.h"
#include "Constraint.h"
#include "Data_List.h"

//tree builder is a friend of all nodes
class Tree_Builder;
class Tree_Node;
class Deci_Tree_Builder;

class Leaf_Node : public Base_Tree_Node
{
friend class Tree_Builder;
friend class Deci_Tree_Builder;
public:

    //constructor
    //creates the leaf with parent constraints and the parent data
    //runs Sample_on_Demand to generate more data as needed based on an entropy confidence measure
    Leaf_Node(const Data_List &nodeData, unsigned int depth, Tree_Node* const parent, int label_);

    //returns class label from the constrained datalist not caring about input
    int Evaluate(const arma::Row<double> &inputExample) const;

    //depth limited overload
    int Evaluate(const arma::Row<double> &inputExample, size_t depthLim) const;

    //Return class label
    int Evaluate() const;

    //returns parent pointer
    Base_Tree_Node* const Get_Parent();

    //returns null
    ChildPair Get_Children();

    //returns 1
    size_t CountLeaf();

    size_t CountLeaf(size_t depth);

    //returns sharedptr to single vector object
    ConstrVecPtr PrintConstr(size_t depth);

    double BalancedModelParity(size_t depthLim, double percent);

    //Returns the nodeData Data_List of all nodes sampled and merged
    Data_List RetSampledDataNode(double percent) const;
  
    //Returns the node sampled data vector pointer
    const std::shared_ptr<DataVec> SampledDataVec(double percent) const;

    double Average_Tree_Accuracy(size_t depthLim, double percent);

    //if this node is not a splittingCandidate it returns 1 or -1, but if it is, it always returns 0
    int Certain_Evaluate(const arma::Row<double> &inputExample, size_t depthLim) const;

    //Counts the number of certain leaf nodes
    size_t CountCertainLeafs() const;
    size_t CountCertainLeafs(size_t depth) const;

    //keeps track if this is a splitting candidate or not
    bool splittingCandidate_;
private:
    //parent of this node
    Tree_Node* const parent_; 

    //On construction, Samples for more data within the constraints to ensure leaves have enough data
    void Sample_Need();

    //Helper function for checking if ref vectors in two points share any refrences
    bool Share_Any_Ref(IntersectionList::const_iterator firstPoint, IntersectionList::const_iterator secondPoint);

    //Accept visitor
    void AcceptVisitor (arma::Row<double> dataPoint, Base_Visitor* const visitor);
};
#endif

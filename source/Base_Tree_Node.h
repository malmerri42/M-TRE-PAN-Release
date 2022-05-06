#ifndef _BASE_TREE_NODE_H_
#define _BASE_TREE_NODE_H_

#include "Data_List.h"
#include "Constraint.h"
#include <memory>
#include <functional>

//Forward declare Base_Visitor
class Base_Visitor;

class Base_Tree_Node
{
friend class Tree_Builder;
public:
    //constructor
    //creates base node containing inputData, and the current depth as parentDepth + 1, handled by parent
    Base_Tree_Node(const Data_List& inputData, unsigned int currentDepth, int label);

    /**
     * Evaluates input data example and returns the class label from the tree by visiting child nodes
     *
     * @param{in]       exinput         Input data to evaluate
     */
    virtual int Evaluate (const arma::Row<double> & exInput) const = 0;

    //depth limited version
    virtual int Evaluate (const arma::Row<double> & exInput, size_t depthLim) const = 0;

    //returns the parent node pointer
    virtual Base_Tree_Node* const Get_Parent() = 0;

    //uses built in Data_List member
    //shows CURRENT entropy
    double Get_Entropy() const;

    //compare the entropy between each other base nodes
    bool operator< (const Base_Tree_Node& RHS);

    //return the children of the current node if they exist (not a leaf), otherwise return null pair
    virtual std::pair<std::shared_ptr<Base_Tree_Node>,std::shared_ptr<Base_Tree_Node>> Get_Children() = 0;

    //depth of node in tree
    const unsigned int depth_;

    //counts the number of leaves in the tree
    virtual size_t CountLeaf() = 0;

    //counts the number of leaves in the tree
    //overloaded for specific depth
    virtual size_t CountLeaf(size_t depth) = 0;

    //prints constraint matrix of all leaf nodes at depth d and above
    typedef std::shared_ptr<std::vector<std::tuple<Constraint, int , const Base_Tree_Node*>>> ConstrVecPtr;
    virtual ConstrVecPtr PrintConstr(size_t depth) = 0;

    //returns copy of constraint
    Constraint GetConstr();

    //Returns Tree Class balanced model parity
    virtual double BalancedModelParity(size_t depthLim, double percent) = 0;

    //Returns the nodeData Data_List of all nodes sampled and merged
    virtual Data_List RetSampledDataNode(double percent) const = 0;
    
    //Returns the node sampled data vector pointer
    virtual const std::shared_ptr<DataVec> SampledDataVec(double percent) const = 0;
    
    //returns the node sampled as a leaf, returns data in datavec form, percentage is how much to sample as a percentage of total sampled for training percent x: 0 < x < 1
    const DataVec& Sample_DataVec_As_Leaf(double percent);
    
    //returns average accuracy of the tree
    virtual double Average_Tree_Accuracy(size_t depthLim, double percent) = 0;

    //samples node and returns accuracy of the node as a leaf
    double Node_Accuracy(double negWeight, double posWeight, double percent);

    //Getter for Data_List used during training 
    const Data_List& GetData_List() const { return nodeData_; };

    //certain evaluate
    //This evaluate function returns 0 if current leaf node is a candidate for splitting, otherwise it calls evaluate normally
    virtual int Certain_Evaluate(const arma::Row<double> &inputExample, size_t depthLim) const = 0;

    //same as count leaf only it counts the number of "certain" leafs
    virtual size_t CountCertainLeafs(size_t depth) const = 0;
    virtual size_t CountCertainLeafs() const = 0;

    //Implement a pseudo visitor pattern (should have done earlier lol)
    //Basically, the visitor is responsible for implementing a depth limited version of it's self, since it can see the depth of the accepter
    //NOTE: Technically the concept of children dont exist in this class
    //Accept visitor take a point to use to traverse the tree
    virtual void AcceptVisitor (arma::Row<double> dataPoint, Base_Visitor* const visitor) = 0;

    //return the value of label_
    int Get_Label() { return label_; }
protected:
    //data stored in this node
    Data_List nodeData_;

    //get model parity COMPONENTS "as a leaf", returns the denominator and numerator in a pair
    //numerator is the first member of the pair, denom second
    std::pair<double,double> As_A_Leaf_Parity_Pair_(double negWeight, double posWeight, double percent);

    //label of node
    const int label_;


    //unlabeled Intersections of the hyperpolygon
    //DataVec innerIntersec_; 

    //validation data stored in this node
    Data_List validNodeData_;
    
};
 
typedef std::pair<std::shared_ptr<Base_Tree_Node>,std::shared_ptr<Base_Tree_Node>> ChildPair;

#include "./Base_Visitor.h"

#endif

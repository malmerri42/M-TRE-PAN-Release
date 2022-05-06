#ifndef _TREE_BUILDER_H_
#define _TREE_BUILDER_H_

#include "Oracle.h"
#include "Leaf_Node.h"
#include "Tree_Node.h"
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "./Local_Exp_Visitor.h"

typedef std::shared_ptr<Base_Tree_Node> BaseNodePtr;
typedef std::shared_ptr<Tree_Node> TreeNodePtr;
typedef std::pair<std::shared_ptr<Leaf_Node>,std::shared_ptr<Leaf_Node>> LeafPtrPair;
typedef std::shared_ptr<Leaf_Node> LeafPtr;

//builds the desicion tree using the oracle to generate data untill the tree is a specified depth
class Tree_Builder
{
public: 

    //initializing constructor
    //supply a file location to load data for the initial Data_List
    //
    //relabel flag to relabel the data according to oracle
    //
    //specify max depth, with 0 being unbounded
    //
    //specify an entropy cutoff for a leaf to be expanded, such that, leaf entropy must be > the cutoff to be expanded
    
    Tree_Builder(const char* file, std::shared_ptr<Oracle> inputOracle, bool relabelFlag = 0, unsigned long maxdepth = 0, double entropyCutoff = 0); 

    //Generates the tree based on the oracle, given some depth (or somehow perfect (or very low gain, for now try perfect) classification)
    BaseNodePtr generate(void);

    //return the tree-model parity 
    //over the provided validation data
    std::tuple<double, double, double, double, double> Model_Parity(double percent);

    double Validate(const char* filename);

    //returns the training accuracy on the whole INITIAL dataset used for training
    double Train_Acc();

    //gets accuracy after normalizing the classes
    double Norm_Train_Acc();

    //prints the direct miss amount
    double MissAmnt();

    //overloaded for a depth
    double MissAmnt(size_t depth);

    //counts leafs down to a depth
    size_t leafCount(size_t depth);
    
    //model parity upto input depth, classBalance class balances the leafs, sampledData weather new data should be sampled and used instead of the validation data from the oracle
    //
    std::tuple<double, double, double, double, double> Model_Parity(size_t depthLim, double percentage);

    //print all the info for each "level" of the tree
    //number of nodes at depth d + leaves below
    //the model parity at each depth level
    //print to file
    virtual void Get_All(double percentValid = 0.50, double margin = 0.05);

    //formatted printing of leaf constraints for analysis at depth
    //This has been repurposed/improved into acting as a global constraint 
    void PrintLeafConstr(size_t depth);

    //return sanitized title
    virtual std::string GetTitle() const;

    //return sanitized filename
    std::string GetFileName() const;
    
    //Finds the model parity on leafs that are certain
    std::tuple<double, double, double, double, double> Certain_Model_Parity(size_t depthLim, double percent);
    
    //Finds certain model pairty on data near the decision boundry of the oracle
    std::tuple<double, double, double, double, double> Boundry_Model_Parity(size_t depthLim, double percent, double margin);

    //returns vector containing pair of ordered matrix of constraint intersection points and the associated label at depth d
    typedef std::vector<std::pair<arma::mat,int>> LeafIntVec;
    void IntersecList(size_t depthLim) const;

    //Get All the intersections for all depths
    void GetAllIntersec() const;

    //return the vector for a dataset, -1 negative, 1 posititve. Can be made to check for uncertain or atleast supply that also, and can be depth limited
    //Just use exsisting base node certain evaluate func
    arma::Col<int> Evaluate(arma::mat& input, size_t depthLim);

    //get the local explanation example:
    //For a dataset, it will provide a local explanation for each point in the form a csv file.
    //The header of the csv will start with the values of the input point and the final MTRE-PAN label for the point and if it is certain or uncertain.
    //This csv will list every trained constraint (not bounding box), listed in chronological order.
    //Each of these constraints will deliniate what the current entropy is (after applying this constraint for the polygon the point belongs to).
    //And how much this current constraint contributed to changing the entropy to its current value after the constraint has been applied
    //Finally the distance from this point to the constraint
    //Assumes Tree is built
    //Has a depth limited version.
    //BE SURE TO MENTION: THIS CAN BE GENERATED VERY EFFICENTLY WITH BRANCH BUILDER
    void LocalExplanation(arma::Mat<double>);//, size_t depthLim);

    //to generate an example for my paper
    //takes the first n of the validation data and returns an explanation
    void Example_LocExp(size_t n);

protected:

    //push leaf nodes
    virtual void PushLeafNodes_(std::stack<LeafPtr>& leafPrioQueue, LeafPtrPair& ptrPair);

    //push root leaf nodes
    virtual void PushRootLeafNodes_(std::stack<LeafPtr>& leafPrioQueue, BaseNodePtr& rootPtr);

    //Expand a leaf node and return its children
    virtual LeafPtrPair Expand_Leaf_(std::shared_ptr<Leaf_Node> inputLeafPtr,std::mutex& cv_m);  

    //comparison helper func for priorty queue, compares base tree nodes entropy values
    //here, 1 is returned if A has a greater entropy (uncertainty)
    bool nodeCompare_( BaseNodePtr A, BaseNodePtr B);
    
    //Helper function to create the tree
    virtual void MakeTree_(const Data_List& inputDataList);

    //the root node in the tree
    BaseNodePtr treeRoot_;

    //maximum depth
    unsigned long maxdepth_;

    //the entropy cutoff to compare against
    double entropyCutoff_;

    //keep a copy pointer to the oracle
    std::shared_ptr<Oracle> oraclePtr_;

    //lazy helper func, but needed in implementation
    void _leafThread_(LeafPtr popedLeaf, std::stack<LeafPtr>& leafPrioQueue, std::vector<std::pair<std::thread,int>> &schedule, const int threadID,  std::mutex& cv_m, std::condition_variable& cv);


    //cond helper func
    bool Cond_(std::stack<LeafPtr>& leafPrioQueue, std::vector<std::pair<std::thread,int>>& schedule, int cond);

    //copy of data stored
    Data_List inputData_;

    //store filename to use in logging
    std::string filename_;

    //Helper func to calculate depth limited confusion mat
    std::tuple<double, double, double, double, double> ConfusionMat_(size_t depthLim, const DataVec& genDataVec, int (Base_Tree_Node::*NodeEval)(const arma::Row<double>&, size_t) const ) const;

    //helper func to find ordered intersections of a given mat Xy - b = 0
    //returns mat of intersection points (x, y, z ...) ordered to produce a polygon as expected by matlab's fill function
    static arma::mat FindInter_(const Constraint& input, const Base_Tree_Node*);

    static bool ValidIntersec_(const arma::Row<double>&, const Constraint&, const Constraint&);
};

#endif

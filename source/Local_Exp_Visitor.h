#ifndef _LOCAL_EXP_VISITOR_H_
#define _LOCAL_EXP_VISITOR_H_

#include "Base_Visitor.h"
#include <stack>
//for arma
#include <mlpack/core.hpp>

class Local_Exp_Visitor : public Base_Visitor
{
public:
    //Constructor takes point we are trying to explain
    //It intializes stream by placing the point as the header (and some other info see debug notes)
    Local_Exp_Visitor(arma::Row<double> point); 
    //the only thing accepters need to know
    //void VisitTreeNode(Tree_Node* const accepter);
    void VisitNode(Base_Tree_Node* const accepterPtr);

    //return ref
    arma::mat& Get_Mat(void){ return data_; }

private:
    //in this instance, the local explanations are placed in a matrix
    arma::mat data_;

    //The data point we are trying to explain
    //Distance is calculated from this
    arma::Row<double> point_;

};


#endif

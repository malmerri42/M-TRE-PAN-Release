#ifndef _BASE_VISITOR_H_
#define _BASE_VISITOR_H_

#include <memory>
#include "./Base_Tree_Node.h"


//Base visitor abstract class
class Base_Visitor
{
public:
    //Constructor 
    //Base_Visitor(){}; 
    //the only thing accepters need to know
    //virtual void VisitTreeNode(Tree_Node* const accepter) = 0;
    virtual void VisitNode(Base_Tree_Node* const accepter) = 0;
};


#endif

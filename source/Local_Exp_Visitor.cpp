#include "Local_Exp_Visitor.h"
#include <sstream>

//initilize points
Local_Exp_Visitor::Local_Exp_Visitor(arma::Row<double> point):
    //Base_Visitor(),
    point_(point)
{
}

void Local_Exp_Visitor::VisitNode(Base_Tree_Node* const accepterPtr)
{
    //IF THIS ISNT A ROOT WE ARE VISITING (look at parent pointer to check)
    if(accepterPtr->Get_Parent() == nullptr) { return; }

    //Make temp data row
    arma::Row<double> dataHolder;

    //Call the method in Base_Tree_Node that gives us it's constraint, and take the last constraint in the list
    Constraint accepterConstraints = accepterPtr->GetConstr();
    arma::mat accepterConstMat = accepterConstraints.GetMat();
    arma::Row<double> accepterSeperator = accepterConstMat.row(accepterConstMat.n_rows - 1);

    //place constraint into data holder
    dataHolder = accepterSeperator;

    //Calculate the distance from this constraint using the point
    // nonAbs_dist = (constraint * transpose(pointVec) - bias)/abs(constraint) (see standard form for bias in constraint.cpp)
    // then dist = abs(nonAbs_dist)
    arma::Row<double> accSepCons = accepterSeperator.cols(0 , accepterSeperator.n_cols - 2);
    double accSepBias = accepterSeperator(accepterSeperator.n_cols - 1);
    double nonAbs_dist = arma::as_scalar((accSepCons * arma::trans(point_) - accSepBias)/arma::norm(accSepCons,2));
    double dist = std::abs(nonAbs_dist);

    //We need to eventually find a normalized average entropy as a result of a split
    //To do this, we need to find the number of data points that fell on each side of a seperator before the nodes were split and had additional data generated for them using Sample_on_demand()
    //specifically, we may need to grab the dataset of the parent and apply it to the constraints of the children to parititon it
    //This data is NOT to be used to calculate entropy, rather it is used to estimate the "size" of the created polygons
    //These following operations are very wasteful can be made more efficent
    Data_List leftData = accepterPtr->Get_Parent()->GetData_List();
    Data_List rightData = accepterPtr->Get_Parent()->GetData_List();
    leftData.Add_Constraint( accSepCons, accSepBias, 1);
    rightData.Add_Constraint( accSepCons, accSepBias, 0);
    //the amount of data is
    double leftDataSize = leftData.Get_Data_Ref().size();
    double rightDataSize = rightData.Get_Data_Ref().size();
    //the proportion of each side in percentage is:
    double leftDataPerc = leftDataSize/(leftDataSize + rightDataSize);
    double rightDataPerc = rightDataSize/(leftDataSize + rightDataSize);

    //find the entropy of the other node (sibling) after splitting
    double sibEntropy;
    //find the sibling
    //Note: for Get_Children, the left child is the .first one
    std::pair<std::shared_ptr<Base_Tree_Node>,std::shared_ptr<Base_Tree_Node>> children = accepterPtr->Get_Parent()->Get_Children();
    //keep track if accepter is the left or right child
    bool accepterIsLeft;
    if(children.first.get() != accepterPtr)
    {
        sibEntropy = children.first->Get_Entropy();
        accepterIsLeft = 0;
    }else
    {
        sibEntropy = children.second->Get_Entropy();
        accepterIsLeft = 1;
    }

    //Prior to applying the seperator, this is the entropy
    double priorEntropy = accepterPtr->Get_Parent()->Get_Entropy();
    //Get the entropy of this nodes data, this is the current value of the entropy after applying the seperator
    double afterEntropy = accepterPtr->Get_Entropy();

    //to save on entropy calculations
    double leftDataEntropy = leftData.Get_Entropy();
    double rightDataEntropy = rightData.Get_Entropy();

    dataHolder = arma::join_horiz(dataHolder, arma::Row<double> 
            {   
                //Prior to applying the seperator, this is the entropy
                priorEntropy,
                
                //Get the entropy of this nodes data, this is the current value of the entropy after applying the seperator
                afterEntropy,

                //This is the entropy of the sibling node
                sibEntropy,

                //Get the change in entropy, by taking the difference between the entropy of this node and it's parent
                (afterEntropy - priorEntropy), 

                //This is the change in entropy from the parent to the sibling
                (sibEntropy - priorEntropy), 

                //This is the relative "size" of this node
                accepterIsLeft ? leftDataPerc : rightDataPerc,

                //This is the relative "size" of the sibling
                !accepterIsLeft ? leftDataPerc : rightDataPerc,

                //This is the weighted entropy after applying the seperator
                leftDataEntropy*leftDataPerc + rightDataEntropy*rightDataPerc,

                //Get the label of this node
                accepterPtr->Get_Label(),

                //distance to the plane
                dist
            } );

    //Append dataHolder to matrix data_
    data_ = arma::join_vert(data_, dataHolder);

}

#include "Constraint.h"
#include <limits>
#include <stack>

//allows the margins to be extended for constraints
#define PERCISION_ALLOWANCE 1e-12

//default Constructor
//empty members
Constraint::Constraint():
    constrMat_(std::make_shared<arma::mat>()),
    biasVec_(std::make_shared<arma::Col<double>>()),
    initFlag_(0)
{

    return;
}

//initializing Constructor
//initialize with row members
/*
Constraint::Constraint(arma::Row<double> inputRow, double bias, bool dLabel):
{


    return;
}
*/

Constraint::Constraint( const Constraint & constr ):
    constrMat_( new arma::Mat<double>(*(constr.constrMat_)) ),
    biasVec_( new arma::Col<double>(*(constr.biasVec_)) ),
    initFlag_(constr.initFlag_)

{

    return;
}


//generate initial bounds
void Constraint::Generate_Initial_(int dim)
{
    //to be safe, use float as limit in a double
    std::numeric_limits<float> floatMaxMin;
    arma::Mat<double> maxBound = arma::eye(dim,dim);
    arma::Col<double> maxBiasBound(dim);
    maxBiasBound.fill( floatMaxMin.max() );

    arma::Mat<double> minBound = -1 * arma::eye(dim,dim);
    arma::Col<double> minBiasBound(dim);
    minBiasBound.fill( -1 * floatMaxMin.lowest() );

    arma::Mat<double> totalBound = arma::join_vert(maxBound, minBound);
    arma::Col<double> totalBias = arma::join_vert(maxBiasBound, minBiasBound);

    //initialize constrMat and biasVec
    constrMat_ = std::make_shared<arma::mat>(totalBound);
    biasVec_ = std::make_shared<arma::Col<double>>(totalBias);
}


//eqFlag is 1 if true examples are along inputRow as a vector (or greater than the line whose weights are inputRow), and 0 otherwise
//Transform all into <=, so if eqFlag == 1 then multiply inputRow and bias by -1
void Constraint::Add_Constraint(arma::Row<double> inputRow, double bias, bool dLabel)
{

    //check if this is an "initial" run, an empty constraint matrix pointer
    //if so initialize with input and numerical limits
    if(!initFlag_)
    {
        //generate numerical limit constraints and intersections
       Generate_Initial_(inputRow.n_cols); 
       initFlag_ = 1;

    }

    //Normalize inputRow
    bias = bias/arma::norm(inputRow);
    inputRow = inputRow/arma::norm(inputRow);

    if(dLabel)
    {
        inputRow = -1 * inputRow;

        bias = -1 * bias;
    }


    //add the constraint to constrMat_ and biasVec_ to use in applying constraint
    *constrMat_ = arma::join_vert(*constrMat_, inputRow);
    (*biasVec_) = arma::join_vert(*biasVec_, arma::Col<double>{bias});

    
    return;
}

        

//overloaded for constraint obj, simply join constrMat and biasVec, input is assumed oldest
/*
void Constraint::Add_Constraint(const Constraint &input)
{
    *(this->constrMat_) = arma::join_cols( *(this->constrMat_) , *(input.constrMat_) );
    *(this->biasVec_) = arma::join_cols( *(this->biasVec_) , *(input.biasVec_) );
}
*/

//overloaded for matrix
void Constraint::Add_ConstraintMat(arma::Mat<double> inputMat, arma::Col<double> biasVec, bool dLabel)
{
    //iterate through rows
    unsigned int rowN = 0;

    for(rowN = 0; rowN < inputMat.n_rows; rowN++)
    {
        //use overloaded function for rows
        Add_Constraint(inputMat.row(rowN), biasVec(rowN,0), dLabel);
    }
}

//return a 1 if a point falls within the constraints, else returns a 0
//this will implement a "fuzziness" constraint check.
bool Constraint::Apply_Constraint( arma::Row<double> inputDataRow ) const
{
    arma::Col<double> multiVec(*constrMat_ * inputDataRow.t());
    arma::Col<double> margin(biasVec_->n_rows);

    //the numerical persicion of a double
    std::numeric_limits<double> doublePercision;

    margin.fill(PERCISION_ALLOWANCE);
    
    //test if within margin, if so then true
    //this does the following constraint - bias <= 0
    //so the assumed standard form is constraint <= bias or constraint = bias
    if( arma::accu((multiVec - *biasVec_)<= margin ) == biasVec_->n_elem )
    {
        return 1;

    }

    return 0;

}

//assignment overwrites current constraint
//seems wrong? check on this
Constraint& Constraint::operator=(const Constraint &RHS)
{
    if(this != &RHS)
    {
        *(this->constrMat_) = *(RHS.constrMat_);
        *(this->biasVec_) = *(RHS.biasVec_);
        this->initFlag_ = RHS.initFlag_;
    }
    
    //derefrenced this
    return *this;
}


void Constraint::PrintDebug()
{
    
    constrMat_->save("cstrMat.csv",arma::csv_ascii);
    biasVec_->save("biasVec.csv",arma::csv_ascii);
}

arma::mat Constraint::GetMat() const
{
    return arma::join_horiz(*constrMat_, *biasVec_);
}

//causes bugs, loses us a lot of constraints, current implementation may create holes
//new idea: does it intersect? NO NEED TO FIND ANY POINTS, just if a h-plane is parallel?
//however a constraint might still not be needed EVEN if they intersect, since the intersection "point" might be far away
//SO, after finding these intersection points between all the constraints they are tested to see if they belong to the constraint. 
//Any plane that has none of its intersections in the plane is considered useless and is to be pruned
void Constraint::PruneConstraint(const DataVec& constrData)
{
    //using one runthough of the constrData, determine which constraints can be pruned
    //A boolean column vector with n rows corrosponding to the number of constraints is created and initialized at 0. 
    //For every point, if a point is closest to a constraint, its entry in the boolean column is changed to 1.
    //at the end, every constraint with entry 0 is deleted
    arma::Col<int> pruneCol (constrMat_->n_rows, arma::fill::zeros); 

    DataVec::const_iterator dataIter = constrData.begin();
    for(;dataIter != constrData.end(); dataIter++)
    {
        arma::Col<double> distCol =  arma::abs((*constrMat_ * dataIter->first.t()) - *biasVec_);
        //normalize each distance
        for(size_t d = 0; d < distCol.n_rows; d++)
        {
            distCol(d) = distCol(d) / arma::norm(constrMat_->row(d), 2);
        }
        
        //set closest constraint to 1, in the prune column
        //Also does is kind to ties, ties for closest are accepted
     // bool done = 0;
     // arma::Col<double> minCol(distCol.n_rows,arma::fill::ones);
     // minCol = minCol * distCol.min();
      //for(size_t k = 0; k < distCol.n_rows; k++)
      //{
      //    //any ties with the min are labeled 1 in the prune Column

      //    //using no approx
      //    if(distCol(k) ==  minCol(k))
      //    {
      //        pruneCol(k) = 1;
      //    }

      //}

        //old super strict, no ties allowed
        pruneCol(distCol.index_min()) = 1;
    }

    //get indicies of pruned constraints
    arma::Col<arma::uword> indexVec;
    //flag to check if prune should happen
    bool prune = 0;
    size_t vecIndex = 0;

  ////the minimum number of constraints needed to enclose a space: dim + 1;
  //const size_t nConstMin(constrMat_->n_cols + 1);
  ////number of constraints
  //size_t nConst (constrMat_->n_rows);

    for(size_t i = 0; i < pruneCol.n_rows; i++)
    {
        if(!pruneCol(i)/* && (nConst > nConstMin)*/)
        {
            //join the index 
            indexVec.insert_rows(vecIndex, arma::Col<arma::uword>{i});  
            vecIndex++; 
            //a prune needs to happen
            prune = 1;

            //lower the number of constraints
            //nConst--;
        }
    }

    if(prune)
    {
        constrMat_->shed_rows(indexVec);
        biasVec_->shed_rows(indexVec);
    }
}

void Constraint::SetInitial(const arma::mat& constrMat, const arma::Col<double>& bias)
{
    constrMat_ = std::make_shared<arma::mat>(constrMat);
    biasVec_ = std::make_shared<arma::Col<double>>(bias);
    initFlag_ = 1;
}

//intialize using max and min rows
void Constraint::SetMaxMinInit(const RowPair& outMaxMin)
{
    //transpose the max and min into columns for the bias
    //and make diag matrices for the constraints
    arma::Col<double> max(outMaxMin.first.t());
    arma::Col<double> min(outMaxMin.second.t());
    
    arma::Mat<double> maxCons = arma::eye(max.n_rows,max.n_rows);
    arma::Mat<double> minCons = arma::eye(min.n_rows,min.n_rows);

    //appliedConstr_.Add_ConstraintMat(maxCons, max, 0);
    //appliedConstr_.Add_ConstraintMat(minCons, min, 1);

    arma::mat comboMat = arma::join_vert(maxCons, -1 * minCons);
    arma::Col<double> comboCol = arma::join_vert(max, -1 * min);

    SetInitial(comboMat, comboCol);
}

#ifndef _CONSTRAINT_H_
#define _CONSTRAINT_H_
//#include <armadillo>
#include <mlpack/core.hpp> //for armadillo
#include <memory>
#include <list>
#include <stack>

//this will be a constraint for the decision tree node, it is a wrapper for a matrix, it accepts vectors of constraints and a bias to add to the current constraint list maintained chronologically

//DataVec.second is the label, 0 means unlabeled
typedef std::pair<arma::Row<double>, int>  DataUnit;
typedef std::vector< DataUnit > DataVec;

typedef std::pair< arma::Row<double>, arma::Row<double> > RowPair;
typedef std::pair< arma::Mat<double>,arma::Col<double> >  MatBiasPair;
typedef std::vector< MatBiasPair > MatBiasPairVec;

//the constraint object holder, a vector of a constraint and the number of refrences to it
typedef std::vector< std::tuple<arma::Row<double>, double> > ConstraintList;

//the list of intersection points, and the list of constraint indexes each of them belong to
typedef std::pair<arma::Row<double>, std::vector<int>> IntersectionPoint;
typedef std::list< IntersectionPoint > IntersectionList;


class Constraint
{
public:
    //default constructor:
    //creates empty list of constraints
    Constraint();

    //(deep) copy constructor
    Constraint( const Constraint & constr );
    
    //initializing constructor
    Constraint(arma::Row<double> inputRow, double bias, bool dLabel);

    //add a constraint as a row vector, double and a bool: Wx - b >< 0
    //this new constraint is added to the end as the "latest" chronological constraint
    //Row vector represents the weights of the inputs
    //double is the bias
    //bool decides if it is greater than or less than (all constraints are converted into less than by multiplying by -1 on both sides)
    void Add_Constraint( arma::Row<double>  inputMat, double bias, bool eqFlag );

    //overloaded constraint obj version
    void Add_Constraint( const Constraint &input );

    //overloaded matrix version 
    void Add_ConstraintMat( arma::Mat<double>  inputRow, arma::Col<double> biasVec, bool eqFlag );

    //Tests constraint, returns 1 if data row falls within the constraints
    //else returns a 0
    bool Apply_Constraint( arma::Row<double>  inputDataRow ) const;

    //debug printer
    void PrintDebug();

    //assignment operator
    Constraint& operator=(const Constraint &RHS);

    //return copy of constrMat_ and biasVec_ joined
    arma::mat GetMat() const;

    //prune constraints using constrData
    //by checking if there exists atleast one point closer to a plane, if a set of planes do not have a point closer to any of them compared to the rest they are to be pruned
    void PruneConstraint(const DataVec &constrData);

    //get-set
    std::shared_ptr<arma::Mat<double>> GetConstrPtr(){ return constrMat_; };
    std::shared_ptr<arma::Mat<double>> GetBiasPtr(){ return biasVec_; };

    //initalize the constraint
    void SetInitial(const arma::mat& constrMat, const arma::Col<double>& bias);

    //Initalize the constraint using max and min
    void SetMaxMinInit(const RowPair& MaxMin);

private:

    //flag to indicate if this has been initialized
    bool initFlag_;

    //should always have used pointers, since implimentation is not known explictly 
    std::shared_ptr<arma::Mat<double>> constrMat_;

    std::shared_ptr<arma::Col<double>> biasVec_;

    //ConstraintList rawConstList_; 

    //helper function that iniitalize with maxbounds
    void Generate_Initial_(int dim);

};
#endif

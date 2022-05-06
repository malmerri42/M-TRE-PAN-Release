#include <iostream>
#include <fstream>
#include <mlpack/core.hpp>

#include <bits/c++config.h>
#include <utility>
#include <vector>
#include <stdio.h>
#include <limits>
#include "MyStats.h"
#include "Ann_Oracle.h"
#include "Circle_Oracle.h"
#include "Sine_Oracle.h"

#include "Deci_Tree_Builder.h"

#include "Branch_Builder.h"


//for debugging
template class arma::Mat<double>;
template class arma::Col<double>;
//template class mlpack::svm::LinearSVM<>; 
template class arma::Row<std::size_t>;
template class arma::Col<size_t>;
template class arma::SpMat<double>;
template class arma::Col<arma::uword>;
template class arma::Col<int>;

void no_cat_adult()
{
    std::shared_ptr<Oracle> oraclePtr1 = std::make_shared<Ann_Oracle> ("../data/census_data/no_cat_adult_data.csv",0.30, 1000);

    Tree_Builder testBuild1("../data/census_data/no_cat_adult_data.csv", oraclePtr1 ,1, 10, 0.0808);
    std::shared_ptr<Base_Tree_Node> ptr1 = testBuild1.generate();
    testBuild1.Get_All();
    
    Deci_Tree_Builder testBuild2("../data/census_data/no_cat_adult_data.csv", oraclePtr1 ,1, 10, 0.0808);
    std::shared_ptr<Base_Tree_Node> ptr2 = testBuild2.generate();
    testBuild2.Get_All();

// Branch_Builder testbuild (oraclePtr1->GetTData().row(0),"../data/census_data/no_cat_adult_data.csv", oraclePtr1 ,1, 100, 0.0808);
// std::shared_ptr<Base_Tree_Node> ptr1 = testbuild.generate();
// testbuild.Get_All();

}


void abalone()
{
    std::shared_ptr<Oracle> oraclePtr1 = std::make_shared<Ann_Oracle> ("../data/abalone_data/abalone_data.csv",0.30, 1000);

  Tree_Builder testBuild3("../data/abalone_data/abalone_data.csv", oraclePtr1 ,1, 10, 0.0808);
  std::shared_ptr<Base_Tree_Node> ptr3 = testBuild3.generate();
  testBuild3.Get_All();
  
  Deci_Tree_Builder testBuild4("../data/abalone_data/abalone_data.csv", oraclePtr1 ,1, 10, 0.0808);
  std::shared_ptr<Base_Tree_Node> ptr4 = testBuild4.generate();
  testBuild4.Get_All();

    //Branch_Builder testbuild (oraclePtr1->GetTData().row(0),"../data/census_data/no_cat_adult_data.csv", oraclePtr1 ,1, 100, 0.00808);
    //std::shared_ptr<Base_Tree_Node> ptr1 = testbuild.generate();
    //testbuild.Get_All();
}

void face_data()
{
    std::shared_ptr<Oracle> oraclePtr3 = std::make_shared<Ann_Oracle> ("../data/grammatical_facial_data/face_data.csv",0.30, 1000);

    Tree_Builder testBuild5("../data/grammatical_facial_data/face_data.csv", oraclePtr3 ,1, 10, 0.0808);
    std::shared_ptr<Base_Tree_Node> ptr5 = testBuild5.generate();
    testBuild5.Get_All();

    Deci_Tree_Builder testBuild6("../data/grammatical_facial_data/face_data.csv", oraclePtr3 ,1, 10, 0.0808);
    std::shared_ptr<Base_Tree_Node> ptr6 = testBuild6.generate();
    testBuild6.Get_All();
}

void spam_data()
{
    std::shared_ptr<Oracle> oraclePtr4 = std::make_shared<Ann_Oracle> ("../data/spam_data/spam_data.csv",0.30, 500);

    Tree_Builder testBuild7("../data/spam_data/spam_data.csv", oraclePtr4 ,1, 10, 0.0808);
    std::shared_ptr<Base_Tree_Node> ptr7 = testBuild7.generate();
    testBuild7.Get_All();
    

    Deci_Tree_Builder testBuild8("../data/spam_data/spam_data.csv", oraclePtr4 ,1, 10, 0.0808);
    std::shared_ptr<Base_Tree_Node> ptr8 = testBuild8.generate();
    testBuild8.Get_All();
}

void challange_data()
{
    std::shared_ptr<Oracle> oraclePtr4 = std::make_shared<Ann_Oracle> ("../data/ADNI_data/ADNI_challenge_data/question_3/GuanLab_solution/filtered_ADNI_challange_q3_trainging_data.csv",0.30, 500);

    Tree_Builder testBuild7("../data/ADNI_data/ADNI_challenge_data/question_3/GuanLab_solution/filtered_ADNI_challange_q3_trainging_data.csv", oraclePtr4 ,1, 5, 0.0808);
    std::shared_ptr<Base_Tree_Node> ptr7 = testBuild7.generate();
    testBuild7.Get_All();
    

    Deci_Tree_Builder testBuild8("../data/ADNI_data/ADNI_challenge_data/question_3/GuanLab_solution/filtered_ADNI_challange_q3_trainging_data.csv", oraclePtr4 ,1, 5, 0.0808);
    std::shared_ptr<Base_Tree_Node> ptr8 = testBuild8.generate();
    testBuild8.Get_All();
}

void cat_adult()// decrease tree size to 5
{
    std::shared_ptr<Oracle> oraclePtr1 = std::make_shared<Ann_Oracle> ("../data/census_data/cat_adult_data.csv",0.30, 100);

    Tree_Builder testBuild1("../data/census_data/cat_adult_data.csv", oraclePtr1 ,1, 10, 0.0808);
    std::shared_ptr<Base_Tree_Node> ptr1 = testBuild1.generate();
    testBuild1.Get_All();
    testBuild1.Example_LocExp(10);

//  Deci_Tree_Builder testBuild2("../data/census_data/cat_adult_data.csv", oraclePtr1 ,1, 6, 0.0808);
//  std::shared_ptr<Base_Tree_Node> ptr2 = testBuild2.generate();
//  testBuild2.Get_All();
//  testBuild2.Example_LocExp(10);

// Branch_Builder testbuild (oraclePtr1->GetTData().row(0),"../data/census_data/no_cat_adult_data.csv", oraclePtr1 ,1, 100, 0.0808);
// std::shared_ptr<Base_Tree_Node> ptr1 = testbuild.generate();
// testbuild.Get_All();

}

void diabetes()
{
    std::shared_ptr<Oracle> oraclePtr1 = std::make_shared<Ann_Oracle> ("../data/diabetes_data/diabetes_data.csv",0.30, 100);

    Tree_Builder testBuild1("../data/diabetes_data/diabetes_data.csv", oraclePtr1 ,1, 10, 0.0808);
    std::shared_ptr<Base_Tree_Node> ptr1 = testBuild1.generate();
    testBuild1.Get_All();
    
//  Deci_Tree_Builder testBuild2("../data/diabetes_data/diabetes_data.csv", oraclePtr1 ,1, 10, 0.0808);
//  std::shared_ptr<Base_Tree_Node> ptr2 = testBuild2.generate();
//  testBuild2.Get_All();

// Branch_Builder testbuild (oraclePtr1->GetTData().row(0),"../data/census_data/no_cat_adult_data.csv", oraclePtr1 ,1, 100, 0.0808);
// std::shared_ptr<Base_Tree_Node> ptr1 = testbuild.generate();
// testbuild.Get_All();

}
void circle_example()
{

    std::shared_ptr<Oracle> oraclePtr1 = std::make_shared<Circle_Oracle> ("../data/synthetic_data/circle_data.csv");
    Tree_Builder testBuild1("../data/synthetic_data/circle_data.csv", oraclePtr1 ,1, 12, 0);
    std::shared_ptr<Base_Tree_Node> ptr1 = testBuild1.generate();
    testBuild1.Get_All();
    testBuild1.GetAllIntersec();

  //Deci_Tree_Builder testBuild2("../data/synthetic_data/circle_data.csv", oraclePtr1 ,1, 12, 0);
  //std::shared_ptr<Base_Tree_Node> ptr2 = testBuild2.generate();
  //testBuild2.Get_All();
  //testBuild2.GetAllIntersec();
}

void sine_example()
{

    std::shared_ptr<Oracle> oraclePtr1 = std::make_shared<Sine_Oracle> ("../data/synthetic_data/sine_data.csv");
    Tree_Builder testBuild1("../data/synthetic_data/sine_data.csv", oraclePtr1 ,1, 12, 0);
    std::shared_ptr<Base_Tree_Node> ptr1 = testBuild1.generate();
    testBuild1.Get_All();
    testBuild1.GetAllIntersec();
    
    Deci_Tree_Builder testBuild2("../data/synthetic_data/sine_data.csv", oraclePtr1 ,1, 12, 0);
    std::shared_ptr<Base_Tree_Node> ptr2 = testBuild2.generate();
    testBuild2.Get_All();
    testBuild2.GetAllIntersec();
}


//example for paper, returns datapoints in the constraint 
void pos_polygon_example()
{
    //initialize constraint
    Constraint myConst;
    arma::rowvec maxRow = {15, 199, 122, 99, 846, 67.1, 2.29, 70};
    arma::rowvec minRow = {0, 56, 0, 0, 0, 0, 0.08, 21};
    myConst.SetMaxMinInit(std::pair<arma::rowvec,arma::rowvec>(maxRow,minRow));

    //add the constraints of the positive polygon
    myConst.Add_Constraint(
            arma::Row<double> {-0.0387,-0.0227945,-0.00509482,-0.00217388,-0.00300178,-0.0230708,-0.99866,0.0090}, -5.28, 0);

    myConst.Add_Constraint(
            arma::Row<double> {-0.0357,-0.0221269,-0.00458512,-0.000124274,-0.00200782,-0.027071,-0.998584,0.0174}, -5.26, 0);


    //create a datalist using the diabetes data
    Data_List myDataList("../data/diabetes_data/diabetes_data.csv");

    //add constraint
    myDataList.Set_Constraint(myConst);
    
    //open file to write
    std::ofstream outFStream;
    outFStream.open("data_in_POS.csv");

    DataVec filteredData = myDataList.Get_Data();
    DataVec::const_iterator constIter = filteredData.begin();
    for(; constIter != filteredData.end(); constIter++)
    {
        const arma::Row<double>& tempRow(constIter->first);
        const int tempLabel(constIter->second);

        for(size_t x = 0; x < tempRow.n_cols; x++)
        {
            outFStream << tempRow(x) << ',';
        }

        outFStream << tempLabel << std::endl;
    }
    outFStream.close();
}

int main()
{

//  cat_adult();

//  std::cout << "cat adult data done!" << std::endl;
//  abalone();

//  std::cout << "abalone data done!" << std::endl;
//  diabetes();

//  std::cout << "diabetes data done!" << std::endl;
    circle_example();

    std::cout << "circle data done!" << std::endl;
//  sine_example();

//  std::cout << "sine data done!" << std::endl;
    pos_polygon_example();

    return 1;
}

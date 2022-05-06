#ifndef _MYSTATS_
#define _MYSTATS_

#include <mlpack/core.hpp> //for armadillo
#include <utility>

class MyStats
{
public:
    //normalize and return the data, and return the max/min for each feature,
    //normalize between a and b: [a, b]
    //both in column major order
    static std::pair<arma::mat,arma::mat> Norm(arma::mat data, double a, double b);

    //Denormalize a line to transform it into the denormalized space
    //by getting the cooefficents using an inverse
    //Needs the  max/min statistics (retrieved from MySats::Norm( ... ).second())
    static arma::Row<double> DeNorm(arma::Row<double> normParams, arma::mat maxMinStats, double a, double b);

};

#endif

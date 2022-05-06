#include "MyStats.h"

std::pair<arma::mat,arma::mat> MyStats::Norm(arma::mat Data, double a, double b)
{
    arma::Row<double> maxDataRow = arma::max(Data,0);
    arma::Row<double> minDataRow = arma::min(Data,0);

    //if maxDataRow and minDataRow have some equal columns then a division by zero will happen, so to avoid it remove the offending columns, then reinsert columns equal to (b + a)/2 midpoint
    //BETTER IDEA: just find non-zero indicies and apply norm to those and zero indicies apply (b + a) / 2
    arma::Row<double> diffRow = maxDataRow - minDataRow;

    arma::uvec nonZeroIndex = arma::find(diffRow);  
    arma::uvec zeroIndex = arma::find(diffRow == 0);  

    arma::mat nonZeroNormData = a + ( (Data.cols(nonZeroIndex) - arma::repmat(minDataRow.cols(nonZeroIndex),Data.n_rows,1))*(b-a)/arma::repmat((maxDataRow.cols(nonZeroIndex) - minDataRow.cols(nonZeroIndex)),Data.n_rows,1) );

    arma::mat normData = Data;

    normData.cols(nonZeroIndex) = nonZeroNormData;
    normData.cols(zeroIndex).fill((b + a) / 2); //= arma::repmat(((b + a) / 2),Data.n_rows, zeroIndex.n_cols);

    std::pair<arma::mat,arma::mat> retPair(normData, arma::join_vert(maxDataRow,minDataRow));

    return retPair;
}

arma::Row<double> MyStats::DeNorm(arma::Row<double> normParams, arma::mat maxMinStats, double a, double b)
{
    //defined max/min rows
    arma::Row<double> maxDataRow = maxMinStats.row(0);
    arma::Row<double> minDataRow = maxMinStats.row(1);

    //define normalized coefficents and bias 
    arma::Row<double> normCoeff = normParams.cols(0,normParams.n_cols - 2);
    arma::Row<double> normBias = normParams.col(normParams.n_cols - 1);

    //find index of zero and nonzero cols
    arma::Row<double> diffRow = maxDataRow - minDataRow;
    arma::uvec nonZeroIndex = arma::find(diffRow);  
    arma::uvec zeroIndex = arma::find(diffRow == 0);  

    //Work on the coefficents and bias seperatly
                                //max - min
    arma::Row<double> nonZeroDeNormCoeff = (normCoeff.cols(nonZeroIndex) * (b - a))/(maxDataRow.cols(nonZeroIndex)- minDataRow.cols(nonZeroIndex));

    arma::Row<double> nonZeroBias = -1 * (b - a) * ( arma::sum( (normCoeff.cols(nonZeroIndex) % minDataRow.cols(nonZeroIndex))/(maxDataRow.cols(nonZeroIndex) - minDataRow.cols(nonZeroIndex))  ,1) )  
        + a * sum(normCoeff.cols(nonZeroIndex), 1) + normBias;

    arma::Row<double> deNormCoeff = normCoeff;
    deNormCoeff.cols(nonZeroIndex) = nonZeroDeNormCoeff;

    arma::Row<double> bias = nonZeroBias + sum( normCoeff.cols(zeroIndex) % ( ((b+a)/2) - maxDataRow.cols(zeroIndex) ) , 1);

    return arma::join_horiz(deNormCoeff,bias);
}

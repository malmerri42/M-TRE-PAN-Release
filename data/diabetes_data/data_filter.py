#!/usr/bin/env python3.8
import pandas as pd
from sklearn import preprocessing

#load data
data = pd.read_csv("./dataset_37_diabetes.arff", 
        dtype={"class":"category"})

#rescale all data to be between two ranges
#max_min_scaler = preprocessing.MinMaxScaler(feature_range=(-1, 1))

#write to csv
#   dataToScale = data.select_dtypes(exclude = ['category']).values
#   dataNumpy = max_min_scaler.fit_transform(dataToScale)
#   dataToWrite = pd.DataFrame(dataNumpy)

labels = data['class'].cat.codes
data = data.drop(columns=['class'])

#dataToWrite.to_csv("census_data_no_cat.csv",
#            index=0)
#labels.to_csv("labels_no_cat.csv",
#            index=0)
#combinedData = pd.concat([dataToWrite,pd.get_dummies(data.select_dtypes(["category"]))], axis = 1)
data['class'] = labels
data.to_csv("diabetes_data.csv",index=0, header=False)

  ##veriData = pd.read_csv("adult.test", 
  ##        skiprows=1,
  ##        names=["age","workclass","fnlwgt","education","education-num","marital-status","occupation","relationship","race","sex","capital-gain","capital-loss","hours-per-week","native-country","income"], 
  ##        dtype={"workclass":"category","education":"category","marital-status":"category","occupation":"category","relationship":"category","race":"category","sex":"category","native-country":"category","income":"category"})

  ##testDataToScale = veriData.select_dtypes(['int64']).values
  ##testDataNumpy = max_min_scaler.fit_transform(testDataToScale)
  ##testDataToWrite = pd.DataFrame(testDataNumpy)

  ##testLabels = veriData['income'].cat.codes
  ###add labels
  ##testDataToWrite["income"] = testLabels
  ##testDataToWrite.to_csv("test_census_data_no_cat.csv",
  ##            index=0, header=False)


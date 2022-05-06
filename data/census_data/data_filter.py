#!/usr/bin/env python3.8
import pandas as pd
from sklearn import preprocessing

#load data
data = pd.read_csv("adult.data", 
        names=["age","workclass","fnlwgt","education","education-num","marital-status","occupation","relationship","race","sex","capital-gain","capital-loss","hours-per-week","native-country","income"], 
        dtype={"workclass":"category","education":"category","marital-status":"category","occupation":"category","relationship":"category","race":"category","sex":"category","native-country":"category","income":"category"})

#drop fnlwgt since it isnt useful across states
data = data.drop(columns=['fnlwgt'])

#encode categorical data
#catData = pd.get_dummies(data)

#rescale all data to be between two ranges
max_min_scaler = preprocessing.MinMaxScaler(feature_range=(-1, 1))

#write to csv
#catData.to_csv("census_data.csv")
#set cat to num codes
labels = data['income'].cat.codes
data = data.drop(columns=['income'])
cat_columns = data.select_dtypes(['category']).columns
data[cat_columns] = data[cat_columns].apply(lambda x: x.cat.codes)
dataToScale = data
dataNumpy = max_min_scaler.fit_transform(dataToScale)
dataToWrite = pd.DataFrame(dataNumpy)


#dataToWrite.to_csv("census_data_no_cat.csv",
#            index=0)
#labels.to_csv("labels_no_cat.csv",
#            index=0)
combinedData = dataToWrite
combinedData['income'] = labels
combinedData.to_csv("cat_adult_data.csv",index=0, header=False)

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


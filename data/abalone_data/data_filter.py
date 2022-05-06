#!/usr/bin/env python3.9
import pandas as pd
from sklearn import preprocessing 

#load data 
data = pd.read_csv("abalone.data",
        names=["Sex", "Length", "Diameter", "Height", "Whole weight", "Shucked weight", "Viscera weight", "Shell weight", "Rings"], 
        dtype={"Sex":"category"},header=None)

#categorize data
catData = pd.get_dummies(data)

#remove the rings column and split into categories based on the median
Rings = catData["Rings"]
catData = catData.drop(columns=["Rings"])
median = Rings.median(axis=0)
Rings[ Rings < median] = 0
Rings[ Rings >= median ] = 1

#normalize catData
max_min_scaler = preprocessing.MinMaxScaler(feature_range=(-1,1))
dataNumpy = max_min_scaler.fit_transform(catData)
catData = pd.DataFrame(dataNumpy)

#merge Rings back to catData
catData["Rings"] = Rings

#print data
catData.to_csv("abalone_data.csv", index=0, header=False)


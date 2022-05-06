%clear all variables
clear;
close all;
%set up RNG
rng('default');

posData = readmatrix("../build/posDataMat.csv").';
negData = readmatrix("../build/negDataMat.csv").';
posLabelData = [posData; zeros(1,size(posData,2)); ones(1,size(posData,2))];
negLabelData = [negData; ones(1,size(negData,2)); zeros(1,size(negData,2))];

oldWeightMat = [1/(size(posData,2)/size(posData,2)) , 0; 0 ,  1/(size(negData,2)/size(posData,2))];
%first new change, 1/ then using the min
weightMat =  [1/(size(negData,2)/size(posData,2)) , 0; 0 , 1/(size(posData,2)/size(posData,2))];

%combined data
allData = [posLabelData, negLabelData];
allData = allData(:,randperm(length(allData)));

dataset = allData(1:2,:);
groundTruth = allData(3:4,:);

delta = 1;
lambda = 0;
    
parameters = [ -0.029877900539809567  , 0.029919038668158298  ;
               0.20072887843615173  , -0.20073320964985814 ;
              -0.33416852179268614  , 0.33417132532568983 ];
          
testParameters = [ -0.019929173822467075  , 0.019970310172919278  ;
               0.063715841654043689  , -0.063720172680565138 ;
              0.38404787077466818  , -0.38404506736282684 ];          

scores = testParameters(1:2,:).' * dataset + testParameters(3,:).';   

%next change, clump the class weights using the ground truth
clumpedClass = [1, 1] * (weightMat * groundTruth);

clumpedScore = [1, 1] * (scores .* groundTruth);
%This says, DONT care about score of correct class, we measure loss as diff
%between incorecct and correct score, if incorrect is higher then loss is
%higher and visa versa

%if delta was equal to 1 when we had only 8800 neg label, what should it be
%when neg label is 149200? it should be multiplied by weightMat!
%still not working: consider what happens when neg data is pruned, what
%changes exactly?

%CURRENTLY: this punishes ALL CLASSES that missclassify as class positive,
%this since we only have 2 classes is members of class NEGATIVE. This is
%incorrect behavior when we have more than 2 classes, since it will punish
%other classes even if they have less memebers than negative. 
%Instead consider using ground truth to distribute class weight
%"punishment"
%to fix it class weight and how it's applied has to change
margin = (scores - [clumpedScore; clumpedScore] + delta - (delta .* groundTruth));

oldLoss = sum(max(oldWeightMat* margin,0), 'all')/(2*8800);
loss = sum(max([clumpedClass; clumpedClass] .* margin,0), 'all')/(sum(clumpedClass));  %assumes equal weight, when above we lower the contribution of negative data points?     
            
Eval = loss;


    
%optimal loss for "balanced" classes
optimalLoss = 0.96242552101310597;

%Data filter generation
% xGzeroP = (posData > 0);
% xGreaterP = posData(:,xGzeroP(1,:));
% xGzeroN = (negData > 0);
% xGreaterN = negData(:,xGzeroN(1,:));
% writematrix(xGreaterP.',"../../M-TRE-PAN_inprogress_weight/source/xGpos.csv");
% writematrix(xGreaterN.',"../../M-TRE-PAN_inprogress_weight/source/xGneg.csv");

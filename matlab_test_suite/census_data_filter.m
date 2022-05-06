%clear all variables
clear;
close all;
%set up RNG
rng('default');

inputData = readmatrix("../data/adult.csv");
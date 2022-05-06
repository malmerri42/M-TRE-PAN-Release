%clear all variables
clear;
%close all;
%set up RNG
rng('default');
%p1x = [0,1,1,0,0,0].';
%p2x = [0,1,1,0,0,0].';

%p1y = [0,0,1,1,1,1].';
%p2y = [1,1,2,2,2,2].';
depth = "12";
model = "mtrepan";
func = "circle";

NegIntrX = readmatrix("../build/intersectionData/x_negative_intersections_" + model + "_" + func + "_data_" + depth + ".csv");
NegIntrY = readmatrix("../build/intersectionData/y_negative_intersections_" + model + "_" + func + "_data_" + depth + ".csv");
UncertIntrX = readmatrix("../build/intersectionData/x_uncertain_intersections_" + model + "_" + func + "_data_" + depth + ".csv");
UncertIntrY = readmatrix("../build/intersectionData/y_uncertain_intersections_" + model + "_" + func + "_data_" + depth + ".csv");
PosIntrX = readmatrix("../build/intersectionData/x_positive_intersections_" + model + "_" + func + "_data_" + depth + ".csv");
PosIntrY = readmatrix("../build/intersectionData/y_positive_intersections_" + model + "_" + func + "_data_" + depth + ".csv");

%clamping
% xLower = -1;
% xUpper = 1;
% yLower = -1;
% yUpper = 1;
% 
% NegIntrX = Clamp(NegIntrX,xLower,xUpper);
% NegIntrY = Clamp(NegIntrY,yLower,yUpper);
% UncertIntrX = Clamp(UncertIntrX,xLower,xUpper);
% UncertIntrY = Clamp(UncertIntrY,yLower,yUpper);
% PosIntrX = Clamp(PosIntrX,xLower,xUpper);
% PosIntrY = Clamp(PosIntrY,yLower,yUpper);
%mymap = [
%    1 0 0;
%    0 1 0;
%    1 1 0
%    ];
    


c = colormap(gray);

for i = 1:size(NegIntrX,2)
    fill(NegIntrX(2:(NegIntrX(1,i) + 1) , i), NegIntrY(2:(NegIntrY(1,i) + 1) ,i), c(100,:), 'LineStyle','none');
    hold on
end
for i = 1:size(UncertIntrX,2)
    fill(UncertIntrX(2:(UncertIntrX(1,i) + 1),i), UncertIntrY(2:(UncertIntrY(1,i) + 1),i), c(175,:), 'LineStyle','none');
    hold on
end
for i = 1:size(PosIntrX,2)
    fill(PosIntrX(2:(PosIntrX(1,i) + 1),i), PosIntrY(2:(PosIntrY(1,i) + 1),i), c(256,:), 'LineStyle','none');
    hold on
end
%edit axes
ax = gca;
ax.LineWidth = 3;
ax.FontSize = 20;
ax.FontUnits = 'normalized';
%fill(pX,pY,pC.');

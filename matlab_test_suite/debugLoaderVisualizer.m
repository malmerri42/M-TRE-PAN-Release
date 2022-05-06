%clear all variables
clear;
close all;
%set up RNG
rng('default');
%debug mat loader/user
posData = readmatrix("../../M-TRE-PAN/build/posDataMat.csv");
negData = readmatrix("../../M-TRE-PAN/build/negDataMat.csv");

%posData = readmatrix("../../M-TRE-PAN/build/posNormDataMat.csv");
%negData = readmatrix("../../M-TRE-PAN/build/negNormDataMat.csv");

%intersec = readmatrix("../../M-TRE-PAN/build/intersecMat.csv");
constraintMat = readmatrix("../../M-TRE-PAN/build/cstrMat.csv");
biasVec = readmatrix("../../M-TRE-PAN/build/biasVec.csv");
xlim([-100, 100])
ylim([-100, 100]) 
for j = 1:size(constraintMat,1)
    yCoeff = constraintMat(j,2);

    if yCoeff == 0
        xline(sign(constraintMat(j,1))*biasVec(j,1)); 
    else
        xCoeff = constraintMat(j,1)./yCoeff;
        bCoeff = biasVec(j,1)./yCoeff;
        refline(-1*xCoeff,bCoeff);
    end


    
end
%margin lines of SVM COME DIRECTLY FROM SVM, SO MUST BE BIAS -1

%   %myline = refline(-1*0.22042455087752613/-2.7871912330810074 ,-1* -2.8233710775578884/-2.7871912330810074);
%   %myline = refline(-1*0.20115416009140322/-1.3524466920307565 ,-1* 3.2590443727235852/-1.3524466920307565 );
%   %myline.Color = 'r';
%   %myline2 = refline(-1*11.946974124222461 /-43.91117353577075, 1*-35.104305708643601 /-43.91117353577075);
%   %myline2 = refline(-1*0.029919038668158298/-0.20073320964985814 ,-1* 0.33417132532568983/-0.20073320964985814 );
%   %myline2.Color = 'g';
%   %xlim([-100, 100])
%   %ylim([-100, 100]) 

   hold on
%   th = 0:pi/50:2*pi;
%   xunit = 1 * cos(th);
%   yunit = 1 * sin(th);
%   h = plot(xunit, yunit);
if size(posData,1) > 0
    scatter(posData(:,1),posData(:,2),5,[0,1,0],'*')
end

if size(negData,1) > 0
    scatter(negData(:,1),negData(:,2),5,[1,0,0],'*')
end
%   scatter(intersec(:,1),intersec(:,2),500,[0,0,1],'*')
%   hold off


%Look into:PYTORCH SVM, TENSOR FLOW, SKLERN.

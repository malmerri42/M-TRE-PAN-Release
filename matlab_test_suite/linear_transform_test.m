%linear transform test
c1 = 1;
c2 = -1;
Q = 2;
N = 1;
k = 0;
M = 0;
b = 0;

slope = -1*(N*c1)/(c2*Q);
bias = -1*(N*c1*k)/(c2*Q) - (N*b)/(c2) - M;
refline(slope,bias);
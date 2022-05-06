function [outputArg] = Clamp(x, bl, bu)
%Clamp clamp value between upper and lower bound
%   Detailed explanation goes here
  outputArg=min(max(x,bl),bu);
end


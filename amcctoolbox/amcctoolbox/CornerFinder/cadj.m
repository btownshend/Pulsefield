function [vout,xout]=cadj(vin,xin)
% CADJ shifts a vector so that the starting value is always the smallest value.
% 
% CADJ shifts the input vector preserving the order in the vector. The
% function ensures that the smallest value is always the first element.
% 
% USAGE:
%     vout=cadj(vin);
% 
%     [vout,xout]=cadj(vin,xin); xout is an adjusted version of xin
%     according to the adjustment of vin and vout
% 
% INPUTS:
%     vin: input vector
% 
%     xin: optional x coordinate vector
% 
% OUTPUTS:
%     vout: adjusted output vector
% 
%     xout: adjusted x coordinate vector
    
[va,loc]=min(vin);
vout=[vin(loc:length(vin)),vin(1:loc-1)];
if nargin>1
    xout=[xin(loc:length(xin)),xin(1:loc-1)];
end
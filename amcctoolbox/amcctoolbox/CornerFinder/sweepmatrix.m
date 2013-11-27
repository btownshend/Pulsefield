function [sweepmatx,sweepmaty]=sweepmatrix(img)
% SWEEPMATRIX precalulates the x and y coordinates for the ray pixels used in CIRCSWEEP.
% 
% SWEEPMATRIX sets up the matrices which when cropped properly can be
% directly used by CIRCSWEEP to find the ray sum results. The main use for
% this function is to improve computational efficiency.
% 
% USAGE:
%     [sweepmatx,sweepmaty]=sweepmatrix(img)
% 
% INPUTS:
%     img: image (the main concern is the size of the image
% 
% OUTPUTS:
%     sweepmatx: x coordinate sweep matrix, each column corresponds to the
%     x coordinates of the pixels that lie under a ray of a certain angle.
%     Angles increase progressively from 0 to 360 along dimension 2 of the
%     array.
% 
%     sweepmaty: y coordinate sweep matrix, each column corresponds to the
%     y coordinates of the pixels that lie under a ray of a certain angle.

theta=pi()/90:pi()/90:2*pi();
win=min(size(img));
r=1:win;
% get grids
[thetag,rg]=meshgrid(theta,r);
sweepmatx=round(rg.*cos(thetag));
sweepmaty=round(rg.*sin(thetag));

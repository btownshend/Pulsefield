function [valid,peaklocs]=validcorner(img,imgedge,sweepmatx,sweepmaty,debug)
% VALIDCORNER checks if the input corner belongs to a chessboard.
% 
% VALIDCORNER is used to indicate if a corner is a chessboard corner. It
% takes as input the cropped image and cropped edge image around the 
% corner. To ensure fast program execution sweepmatx and sweepmaty have
% been used. They are necessary input to this function.
% 
% USAGE:
%     [valid,peaklocs]=validcorner(img,imgedge,sweepmatx,sweepmaty);
% 
% INPUTS:
%     img: cropped grayscale image
% 
%     imgedge: cropped Sobel edge image
% 
%     sweepmatx: cropped sweepmatrix x, this is used for fast radial summation
% 
%     sweepmatx: cropped sweepmatrix y, this is used for fast radial summation
% 
% 
% OUTPUTS
%     valid: scalar indicating wether the point is a chessboard corner of not, 1: yes, 0: no
% 
%     peaklocs: required by GETGRID

% validcorner parameters
imadjsca=0.8; % larger adjust scalars corresponds to less adjustment
imeadjsca=1.8;
intth=0.5;


if ~exist('debug','var') || isempty(debug)
    debug=0;
end

peaklocs=[];

if debug
    close all;
end

%Adjust windowed image
imgedge=adjimg(imgedge,imeadjsca); %edge images need less adjustment than imgn

if debug
    figure;imshow(imgedge);
end

[theta,edgevalue,thetasmd,edgevaluesmd]=circsweep(imgedge,sweepmatx,sweepmaty);



maxtab=peakdet(edgevalue);


if size(maxtab,1)~=4      %Check if peaks equal 4
    valid=0;
    return;
end

peaklocs=maxtab(:,1);

maxtabsmd=peakdet(edgevaluesmd);
if size(maxtabsmd,1)~=2      %Check if peaks equal 2
    valid=0;
    return;
end


img=adjimg(img,imadjsca);
[theta,intvalue,thetasmd,intvaluesmd]=circsweep(img,sweepmatx,sweepmaty);
% intth=(max(intvaluesmd)-min(intvaluesmd))/2;

%Work with summed arrays
peaks=maxtabsmd(:,2);
locs=maxtabsmd(:,1);


peaks=peaks';
locs=locs';

% sort peak locations
locs=sort(locs);


crn1=mean(intvaluesmd([1:locs(1),locs(2):length(intvaluesmd)]));
crn2=mean(intvaluesmd(locs(1):locs(2)));


%Check if squares have enough intensity difference
if abs(crn1-crn2)>intth
    valid=1;
else
    valid=0;
end
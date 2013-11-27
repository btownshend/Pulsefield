function crnrpts=getcrnrpts(imgh,mimg,stdv,th,debug)
% GETCRNRPTS is a function that uses the mean and standard deviation images to adaptively obtain local maxima in an image.
% 
% GETCRNRPTS can be applied on the Harris transform to obtain the Harris
% corner points.
% 
% USAGE:
%     crnrpts=getcrnrpts(imgh,mimg,stdv,th);
% 
% INPUTS:
%     imgh: Harris transform of image
% 
%     mimg,stdv: output from ADAPTSTATS
% 
%     th: parameter to adjust thresholding
% 
% OUTPUTS:
%     crnrpts: 2xN array with coordinates of corner points

% Check input
if ~exist('debug','var') || isempty(debug)
    debug=0;
end

% adaptive thresholding
imax=mimg+th*stdv;
imax(imax>1)=1;
imax(imax<0)=0;
imghl=0*imgh;
imghl(imgh>imax)=1;
imghl=logical(imghl);
imghlf=medfilt2(imghl);
imghlf=medfilt2(imghlf);

% get centroids of blobs as Harris corner points
[imglfl,n]=bwlabel(imghlf);
meanx=zeros(1,n);
meany=zeros(1,n);
cnt=zeros(1,n);


[x,y,v]=find(imglfl);

for cntr=1:size(x)
    meanx(v(cntr))=meanx(v(cntr))+x(cntr);
    meany(v(cntr))=meany(v(cntr))+y(cntr);
    cnt(v(cntr))=cnt(v(cntr))+1;
end

meanx=round(meanx./cnt);
meany=round(meany./cnt);
crnrpts=[meanx;meany];

% debugging
if debug
    close all;
    figure;imshow(imghl);
    figure;imshow(imghlf);
    pause;
end
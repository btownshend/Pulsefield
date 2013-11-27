function [mimg,stdv]=adaptstats(img,win)
% ADAPTSTATS gets the local mean and standard deviations for each pixel in the image within a region.
% 
% USAGE:
%     [mimg,stdv]=adaptstats(img,win);
%     [mimg,stdv]=adaptstats(img);
% 
% INPUTS:
%     img: grayscale image of class double
% 
%     win: (optional) default value is min(size(img))/5, win is the size of
%     the region

%Check input
if ~exist('win','var') || isempty(win)
    % default win size
    win=round(min(size(img))/5);
end

% create integral image, integral image allows for quicker execution time
% for large window sizes.
intimg = cumsum(cumsum(img),2);
intimg2 = cumsum(cumsum(img.^2),2);

hwin=floor(win/2);

% pad integral images

intimg=padarray(intimg,[hwin+1,hwin+1],'pre');
intimg=padarray(intimg,[hwin,hwin],'post','replicate');
intimg2=padarray(intimg2,[hwin+1,hwin+1],'pre');
intimg2=padarray(intimg2,[hwin,hwin],'post','replicate');

% get mean and stdv images
simg=intimg(1:size(img,1),1:size(img,2))+intimg(win+1:size(img,1)+win,win+1:size(img,2)+win)...
    -intimg(1:size(img,1),win+1:size(img,2)+win)-intimg(win+1:size(img,1)+win,1:size(img,2));
simg2=intimg2(1:size(img,1),1:size(img,2))+intimg2(win+1:size(img,1)+win,win+1:size(img,2)+win)...
    -intimg2(1:size(img,1),win+1:size(img,2)+win)-intimg2(win+1:size(img,1)+win,1:size(img,2));

n=win^2;
mimg=simg/n;
vari=(simg2-(mimg.^2)*n)/(n-1);
vari=max(0,vari);   % BT-2013/11/25: Hit a bug where some elements of vari were less than 0; kludge here to prevent sqrt of negatives
stdv=sqrt(vari);
function imgout=adjimg(img,th)
% ADJIMG adjusts the intensity of the input image.
% 
% ADJIMG adjusts the intensity of the input image based on the mean and the
% standard deviation of the intensitites in the image.
% 
% USAGE:
%     imgout=adjimg(img); default values are used
% 
%     imgout=adjimg(img,th); th tunes the adjustment, higher th values
%     results in less adjustment
% 
% INPUTS:
%     img: input grayscale image of double class
% 
%     th: tuning parameter
% 
% OUTPUTS:
%     imgout: adjusted image



if nargin<2 || isempty(th)
    th=1;
end
mimg=mean2(img);
stdv=std2(img);

imax=mimg+th*stdv;
imin=mimg-th*stdv;

if imax>1
    imax=1;
end
if imin<0
    imin=0;
end
imgout=(img-imin)./(imax-imin);
imgout(imgout>1)=1;
imgout(imgout<0)=0;
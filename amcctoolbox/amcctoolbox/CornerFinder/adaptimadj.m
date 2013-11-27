function imgout=adaptimadj(img,win,th)
% ADAPTIMADJ adaptively adjusts the intensity of an image
% 
% ADAPTIMADJ adaptively adjusts the intensity of the input image. The
% adjustment is performed according to the mean and standard deviation of
% the region around each pixel.
% 
% USAGE:
%     imgout=adaptimadj(img); default window size is min(size(img))/5,
%     default th is 1.
% 
% INPUTS:
%     img: grayscale double image
% 
%     win: size of the rectangular region to inspect at each pixel
% 
%     th: parameters that tunes the degree of adjustment
% 
% OUTPUTS:
%     imgout: adjusted image

% check input
if nargin<2 || isempty(win);
    win=round(min(size(img))/5);
end
if nargin<3 || isempty(th)
    th=1;
end


[mimg,stdv]=adaptstats(img,win);

imax=mimg+th*stdv;
imin=mimg-th*stdv;

% clip
imax(imax>1)=1;
imin(imin<0)=0;

imgout=(img-imin)./(imax-imin);

% handle case when stdv is zero for a given pixel (results in DIV0 -> nan)
imgout( ~isfinite(imgout) ) = 0;

% adjust for clipping and saturation
imgout(imgout>1)=1;
imgout(imgout<0)=0;
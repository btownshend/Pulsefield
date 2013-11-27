function imgout=harristm(img,win)
% HARRIS obtains the Harris transform of image.
% 
% HARRIS takes gets the Harris transform image of an input grayscale image.
% 
% USAGE:
%     imgout=harris(img); if win is not specified the default value is
%     used min(size(img))/140
% 
%     imgout=harris(img,win); win is the window size of the Harris
%     transform
% 
% INPUTS:
%     img: grayscale double class image
% 
%     win: scalar specifying the window size
% 
% OUTPUTS:
% imgout: Harris transform image

if ~exist('win','var')|| isempty(win)
    win=round(min(size(img))/140);
end

dx =[-1 0 1; -2 0 2;-1 0 1]; % The Mask 
    dy = dx';

    
    ix = conv2(img, dx, 'same');   
    iy = conv2(img, dy, 'same');
    m = fspecial('average',win);
    

    a = conv2(ix.^2, m, 'same');  
    b = conv2(iy.^2, m, 'same');
    c = conv2(ix.*iy,m,'same');

    imgout = (a.*b - c.^2)./(a + b + eps);
    imgout=gscale(imgout,'minmax');

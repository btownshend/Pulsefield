function [imge,ix,iy]=getedges(img)
% GETEDGES gets the Sobel edge image.
% 
% USAGE:
%     imge=getedges(img)
% 
%     [imge,ix,iy]=getedges(img)
% 
% INPUTS:
%     img: grayscale double image
% 
% OUTPUTS:
%     imge: the edge image
% 
%     ix: x component gradient image
% 
%     iy: y component gradient image

dx =[-1 0 1; -2 0 2;-1 0 1];
dy = dx';
ix = conv2(img, dx, 'same');   
iy = conv2(img, dy, 'same');

imge=gscale(sqrt(ix.^2+iy.^2),'minmax');
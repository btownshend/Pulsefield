function win=getwin(img,pt,crnrpts)
% GETWIN chooses an appropriate window size for the chessboard corner filter
% 
% GETWIN chooses the window size depending on the spread of the points. It
% simply chooses a window size which does not include any other points.
% 
% USAGE:
%     win=getwin(img,pt,crnrpts)
% 
% INPUTS:
%     img: grayscale image (only used for size)
% 
%     pt: coordinates of pixel of interest
% 
%     crnrpts: 2xN array of all other points
% 
% OUTPUTS:
%     win: window size


x=pt(1);
y=pt(2);
nearestpt=findnearest([x;y],crnrpts,1);
win=max(abs(nearestpt-[x;y]))-2;

% check for borders
if x-win<1
    win=x-1;
end
if x+win>size(img,1)
    win=size(img,1)-x;
end
if y-win<1
    win=y-1;
end
if y+win>size(img,2)
    win=size(img,2)-y;
end

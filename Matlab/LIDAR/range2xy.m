% Convert angle+range to x,y
% FIXME: Doesn't consider LIDAR translation, rotation
function xy=range2xy(angle,range)
angle=angle+pi/2;
xy=zeros(length(range),2);
[xy(:,1),xy(:,2)]=pol2cart(angle(:),range(:));

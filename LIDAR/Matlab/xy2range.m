% Convert angle+range to x,y
function [angle,range]=xy2range(xy)
[angle,range]=cart2pol(xy(:,1),xy(:,2));
angle=angle-pi/2;

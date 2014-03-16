% Convert angle+range to x,y
function xy=range2xy(angle,range)
angle=angle+pi/2;
xy=zeros(size(range,3),2);
[xy(:,1),xy(:,2)]=pol2cart(angle(:),squeeze(range(1,1,:)));

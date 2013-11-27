function [nxpts,ixs]=findnearest(p,pts,num,same)
% FINDNEAREST finds from a array of points the nearest to a certain point.
% 
% FINDNEAREST finds the nearest num points to point p from points pts. If
% same is input, the same point will be returned if it exists in pts.
% 
% USAGE:
%     [nxpts,ixs]=findnearest(p,pts,num); gets the nearest num pts
% 
%     [nxpts,ixs]=findnearest(p,pts,num,1); the same point is returned if
%     it exists
% 
% INPUTS:
%     pt: reference point
% 
%     pts: 2xN array of points
% 
%     num: number of nearest points required
% 
%     same: flag, if input allows the function to return point pt
% 
% OUTPUTS:
%     nxpts: 2xnum array containing the found points
% 
%     ixs: the column coordinates of the points

if size(pts,1)>2
    pts=pts';
end
dist=sqrt((p(1)-pts(1,:)).^2+(p(2)-pts(2,:)).^2);
% if nargin<4
%     ind=find(dist==0,1);
%     if ~isempty(ind)
%         pts(:,ind)=[NaN;NaN];
%         dist(ind)=NaN;
%     end
% end

[dist,ix]=sort(dist);

if nargin<4
    ind=find(dist==0,1);
    ix(ind)=[];
end
nxpts=pts(:,ix(1:num));
ixs=ix(1:num);
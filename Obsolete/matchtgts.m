% Update current hyothesis of locations 
% Input:
%   hprob(k,isize,isize) - for each individual, k, a map of the prob they are centered at the given coord (1=possibly there, 0=not there)
%   unblocked(isize,isize) - pixel positions that could be a center point as they are not obstructed by anything within a certain radius
%   maxchange - max movement of anyone in pixels
% Output:
%   hfinal - updated hprob
function hfinal=matchtgts(p,hprob,unblocked,maxchange,doplot)
if nargin<5
  doplot=0;
end
pghost=0.5;   % Prob any given pixel is a ghost pixel
% Build a filter to allow moment
filt=fspecial('gaussian',sdev);

for i=1:size(hprob,1)
  hfinal(i,:,:)=imdilate(hprob(i,:,:),se) & ~unblocked;
end

% Scale to account for multiple targets being at the same point
hfinal=
% Compute distances between all pairs
for i=1:size(tpos1,1)
  dist(i,:)=(tpos1(i,1)-tpos2(:,1)).^2+(tpos1(i,2)-tpos2(:,2)).^2;
end
map=dist<maxchange;
used=zeros(1,size(tpos2,1));
for a=1:size(tpos1,1)
  sel1=find(~used);
  sel2=dist(a,sel1)<maxchange;
  sel=sel1(sel2);
  if length(sel)==0
    % Keep old estimate
    hfinal(a,:)=h(a,:);
  elseif length(sel)==1
    % New position
    hfinal(a,:)=tpos2(sel,1);
    used(sel)=1;
  elseif length(sel)>1
    % Split - multiple new positions possible
    % Use closest unused one
    [mindist,e]=min(dist(a,sel));
    hfinal(a,:)=tpos2(sel(e),:);
    used(sel(e))=1;
  end
end
% Print remaining ones
remain=find(used==0);
for i=1:length(remain)
  hfinal(end+1,:)=tpos2(remain(i),:);
end

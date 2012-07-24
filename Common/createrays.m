% createrays - create rays in setup for use in finding targets
%   layout - positions of cameras/leds/etc in meters
%   isize - number of pixels high/wide mapped image should be
% Returns
%   r.rays{c}(x,y) - image of region with each pixel indicating LED# that ray from this camera hits after passing through this pixel, or 0 for none
%   r.raylines{c,l} - path (in integer pixel coordinates) of ray from camera c to led l, stepping by 1 pixel in fastest changing direction
function r=createrays(layout,pixels)
imap=setupimap(layout,pixels);   % Setup mapping between pixel and physical coords
cpos=m2pix(imap,layout.cpos);
lpos=m2pix(imap,layout.lpos);

% Create rays (image with each pixel indicating the highest LED number that the ray is directed to)
% Also create raylines{c,l}(i,2) - coords of points along ray from camera c to led l
raylines={};
for c=1:size(layout.cpos,1)
  fprintf('Computing rays from camera %d\n',c);
  rays=zeros(imap.isize(1),imap.isize(2));
  for l=1:size(layout.lpos,1)
    raylines{c,l}=zeros(0,2);
    cp=cpos(c,:);
    lp=lpos(l,:);
    delta=lp-cp;
    maxd=max(abs(delta));
    delta=delta/maxd;
    pv=cp;
    while (pv(1)>imap.isize(1) || pv(2)>imap.isize(2) || min(pv)<0) && maxd>0
      pv=pv+delta;
      maxd=maxd-1;
    end
    while pv(1)<imap.isize(1) && pv(2)<imap.isize(2) && min(pv)>=0 && maxd>0
      fp=floor(pv)+1;
      rays(fp(1),fp(2))=l;
      raylines{c,l}(end+1,:)=fp;
      pv=pv+delta;
      maxd=maxd-1;
    end
  end
  r.rays{c}=rays;
  rsel=r.rays{c}~=0;
  r.nzrayindices{c}=find(rsel);
  r.nzraymap{c}=r.rays{c}(rsel);
end
r.raylines=raylines;
r.imap=imap;

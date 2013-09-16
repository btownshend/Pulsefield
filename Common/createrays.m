% createrays - create rays in setup for use in finding targets
%   layout - positions of cameras/leds/etc in meters
%   isize - number of pixels high/wide mapped image should be
% Returns
%   r.rays{c}(x,y) - image of region with each pixel indicating LED# that ray from this camera hits after passing through this pixel, or 0 for none
%   r.raylines{c,l} - path (in integer pixel coordinates) of ray from camera c to led l, stepping by 1 pixel in fastest changing direction
function r=createrays(p,useanglemap)
if nargin<2
  useanglemap=false;
end
pixels=p.analysisparams.npixels;
imap=setupimap(p.layout,pixels);   % Setup mapping between pixel and physical coords
cpos=m2pix(imap,p.layout.cpos);
lpos=m2pix(imap,p.layout.lpos);

% Create rays (image with each pixel indicating the highest LED number that the ray is directed to)
% Also create raylines{c,l}(i,2) - coords of points along ray from camera c to led l
raylines={};
for c=1:size(p.layout.cpos,1)
  fprintf('Computing rays from camera %d\n',c);
  rays=zeros(imap.isize(1),imap.isize(2));
  for l=1:size(lpos,1)
    raylines{c,l}=zeros(0,2);
    cp=cpos(c,:);
    if useanglemap
      ppos=p.camera(c).pixcalib(l).pos;
      angle=-p.camera(c).anglemap(ppos(1));   % Note that the anglemap angles increase in CW direction, but pol2cart works in reverse
      dir=[];
      [dir(1),dir(2)]=pol2cart(cart2pol(p.layout.cdir(c,1),p.layout.cdir(c,2))+angle,1);
      lp=cp+dir*pixels*2;
      r.raydir(c,l,:)=dir;
    else
      lp=lpos(l,:);
    end
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

% Points where someone could possibly be
% Polygon in pixel coords
active=m2pix(imap,p.layout.active);
% Convert to mask (need to transpose since masks are usually (y,x) addressed
r.activemask=poly2mask(active(:,1),active(:,2),imap.isize(1),imap.isize(2))';

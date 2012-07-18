% Analyze sensor data to determine positions of targets
% Input:
%  p - setup
%  layout - actual or estimated positions of cameras, leds
%  v(c,l) - 1 if led l is visible to camera c, 0 if not visible, nan if not within field of view
% Output:
%  possible - image with 1 where someone could possibly be, 0 where they are definitely not
%  tgts - structure of possible targets
%    tpos - center of target
%    maxdiam - maximum possible diameter of target
%    nunique - number of rays blocked that could only be attributed to this target
function [possible,tgts]=analyze(p,layout,v,rays,doplot,truetgts)
if nargin<5
  doplot=false;
end
% im is zero where someone could be, 1 where they can't be
im=zeros(rays.imap.isize);

% Points where someone could possibly be
% Polygon in pixel coords
active=m2pix(rays.imap,layout.active);
% Convert to mask (need to transpose since masks are usually (y,x) addressed
ai=poly2mask(active(:,1),active(:,2),size(im,1),size(im,2))';
if doplot>2
  setfig('ai');
  imshowmapped(rays.imap,ai);
  title('active area');
end

% Assume there isn't anyone outside
im=~ai;

% Draw all visible rays
if doplot>1
  cnt0=zeros(size(im));
  cnt1=zeros(size(im));
end
for c=1:size(v,1)
  v1=find(v(c,:)==1);
  isone=ismember(rays.rays{c},v1);
  im=im | isone;
  if doplot>1
    cnt1(isone)=cnt1(isone)+1;
    %  fprintf('c=%d, cnt0(500,500)=%d, cnt1=%d\n', c, cnt0(500,500), cnt1(500,500));
    iszero=ismember(rays.rays{c},find(v(c,:)==0));
    cnt0(iszero)=cnt0(iszero)+1;
  end
end

% Convert target diameters into (float) pixel coords
mintgtdiampix=m2pix(rays.imap,p.analysisparams.mintgtdiam);

% Dilate image using disk
% structure element slightly smaller than a person
shrink=floor(mintgtdiampix/2);
se=strel('disk',shrink,0);
possible=imerode(im==0,se);	% Erode image (1==center of person could be here)

% Use a structuring element slightly smaller than the one used to erode so that the regions don't end up merging together
sereduced=strel('disk',floor(mintgtdiampix/2)-1,0);
ibd=imdilate(possible,sereduced);	% Regrow it (1==person could be present)
[lbl,nobjs]=bwlabel(ibd);	% Label it with object numbers

% Figure out centers of each possible target
stats=regionprops(lbl','Centroid','Area','MinorAxisLength','MajorAxisLength');  % Use transpose to go back to coord axis
% Create structure of results
tgts=[];
for i=1:length(stats)
  pos=pix2m(rays.imap,stats(i).Centroid);
  minoraxislength=(stats(i).MinorAxisLength+2)/rays.imap.scale;
  majoraxislength=(stats(i).MajorAxisLength+2)/rays.imap.scale;
  area=((sqrt(stats(i).Area)+2)/rays.imap.scale)^2;
  tgts=[tgts,struct('pos',pos,'minoraxislength',minoraxislength,'majoraxislength',majoraxislength,'area',area,'stats',stats(i));
end
        
if doplot>1
  setfig('analyze.rays');
  clf;
  imc=zeros(size(im,1),size(im,2),3);
%  imc(:,:,1)=cnt0/size(v,1);
  imc(:,:,2)=cnt1/size(v,1);
  imshowmapped(rays.imap,imc);
  title('analyze.rays');
  hold on;
  if exist('truetgts','var')
    viscircles(truetgts.tpos,truetgts.tgtdiam/2,'LineWidth',0.5);
  end
  for i=1:length(tgts)
    plot(tgts(i).pos(1),tgts(i).pos(2),'or');
    text(tgts(i).pos(1),tgts(i).pos(2),sprintf('%d',i),'color','r','HorizontalAlignment','center');
  end
  for i=1:size(layout.cpos,1)
    plot(layout.cpos(i,1),layout.cpos(i,2),'og');
    text(layout.cpos(i,1),layout.cpos(i,2),sprintf('%d',i),'HorizontalAlignment','center');
  end
end

% Compute which points are possible ghosts
tgts.nunique=0;
[cind,lind]=ind2sub(size(v),find(v(:)==0));
for i=1:length(cind)
  c=cind(i);
  l=lind(i);
  %      profile=improfile(lbl,rays.raylines{c,l}(:,2),rays.raylines{c,l}(:,1));
  profile=lbl(sub2ind(size(lbl),rays.raylines{c,l}(:,1),rays.raylines{c,l}(:,2)));
  profile=profile(profile~=0);
  if length(profile)>0 && all(profile==profile(1))
    % Unique object
    tgts(profile(1)).nunique=tgts(profile(1)).nunique+1;
  end
end

if doplot>0
  setfig('analyze.estimates');
  clf;
  imshowmapped(rays.imap,(1-ibd/2));
  title('analyze.estimates');
  hold on;
  if exist('truetgts','var')
    viscircles(truetgts.tpos,truetgts.tgtdiam/2,'LineWidth',0.5);
  end
  rp=regionprops(lbl,'Area','Centroid');
  for i=1:length(tgts)
    cent=pix2m(rays.imap,rp(i).Centroid([2,1]));
    plot(cent(1),cent(2),'or');
    text(cent(1),cent(2),sprintf('%d (%d)',i,tgts(i).nunique),'color','k','HorizontalAlignment','center');
    fprintf('Object %d at (%.1f,%.1f) was unique for %d rays\n', i, cent, tgts(i).nunique);
  end
end


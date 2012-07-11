% Analyze sensor data to determine positions of targets
% Input:
%  p - setup
%  layout - actual or estimated positions of cameras, leds
%  v(c,l) - 1 if led l is visible to camera c, 0 if not visible, nan if not within field of view
% Output:
%  possible - image with 1 where someone could possibly be, 0 where they are definitely not
%  tgts - structure of targets
%    center - center of target
%    maxdiam - maximum possible diameter of target
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
end
cnt1=zeros(size(im));
for c=1:size(v,1)
  sr=squeeze(rays.rays(c,:,:));
  v1=find(v(c,:)==1);
  isone=ismember(sr,v1);
  im=im | isone;
  cnt1(isone)=cnt1(isone)+1;
  %  fprintf('c=%d, cnt0(500,500)=%d, cnt1=%d\n', c, cnt0(500,500), cnt1(500,500));
  if doplot>1
    iszero=ismember(sr,find(v(c,:)==0));
    cnt0(iszero)=cnt0(iszero)+1;
  end
end

% Convert target diameters into (float) pixel coords
mintgtdiampix=m2pix(rays.imap,p.analysisparams.mintgtdiam);

% Dilate image using disk
% structure element slightly smaller than a person
se=strel('disk',floor(mintgtdiampix/2),0);
possible=imerode(im==0,se);	% Erode image (1==center of person could be here)

% Figure out centers of each possible target
[cx,cy]=ind2sub(size(possible),find(bwmorph(possible,'shrink',inf)));

% Convert back to meters for everything from here on
tgts.tpos=pix2m(rays.imap,[cx,cy]);

if doplot>1
  setfig('analyze.rays');
  clf;
  imc=zeros(size(im,1),size(im,2),3);
  imc(:,:,1)=cnt0/size(v,1);
  imc(:,:,2)=cnt1/size(v,1)/4;
  imshowmapped(rays.imap,imc);
  title('analyze.rays');
  hold on;
  if exist('truetgts','var')
    viscircles(truetgts.tpos,truetgts.tgtdiam/2,'LineWidth',0.5);
  end
  for i=1:size(tgts.tpos,1)
    plot(tgts.tpos(i,1),tgts.tpos(i,2),'or');
    text(tgts.tpos(i,1),tgts.tpos(i,2),sprintf('%d',i),'color','r','HorizontalAlignment','center');
  end
  for i=1:size(layout.cpos,1)
    plot(layout.cpos(i,1),layout.cpos(i,2),'og');
    text(layout.cpos(i,1),layout.cpos(i,2),sprintf('%d',i),'HorizontalAlignment','center');
  end
end

if doplot>0
  % Compute which points are possible ghosts
  ibd=imdilate(possible,se);	% Regrow it (1==person could be present)
  [lbl,nobjs]=bwlabel(ibd);	% Label it with object numbers
  uobjs=zeros(1,nobjs);
  nobjs=[];
  for c=1:size(rays.raylines,1)
    for l=1:size(rays.raylines,2)
      if v(c,l)==0
        profile=improfile(lbl,rays.raylines{c,l}(:,2),rays.raylines{c,l}(:,1));
        nobjs(c,l)=length(unique(profile))-1;
        if nobjs(c,l)==1
          % Unique object
          uobjs(max(profile))=uobjs(max(profile))+1;
        end
      end
    end
  end

  setfig('analyze.estimates');
  clf;
  imshowmapped(rays.imap,(1-ibd/2));
  title('analyze.estimates');
  hold on;
  if exist('truetgts','var')
    viscircles(truetgts.tpos,truetgts.tgtdiam/2,'LineWidth',0.5);
  end
  rp=regionprops(lbl,'Area','Centroid');
  for i=1:length(uobjs)
    cent=pix2m(rays.imap,rp(i).Centroid([2,1]));
    plot(cent(1),cent(2),'or');
    text(cent(1),cent(2),sprintf('%d (%d)',i,uobjs(i)),'color','k','HorizontalAlignment','center');
    fprintf('Object %d at (%.1f,%.1f) was unique for %d rays\n', i, cent, uobjs(i));
  end
end


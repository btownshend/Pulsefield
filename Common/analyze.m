% Analyze sensor data to determine positions of targets
% Input:
%  p - setup
%  v(c,l) - 1 if led l is visible to camera c, 0 if not visible, nan if not within field of view
% Output:
% snap:
%  possible - image with 1 where someone could possibly be, 0 where they are definitely not
%  tgts - structure of possible targets
%    tpos - center of target
%    maxdiam - maximum possible diameter of target
%    nunique - number of rays blocked that could only be attributed to this target
function snap=analyze(p,v,doplot,truetgts)
if nargin<3
  doplot=false;
end
if isfield(p.analysisparams,'expandrays')
  v=expandrays(p,v);
end
layout=p.layout;
rays=p.rays;
% im is zero where someone could be, 1 where they can't be
im=zeros(rays.imap.isize);

if ~isfield(rays,'activemask')
  error('p.rays.activemask missing: need to rerun p.rays=createrays(p)');
end
if 0
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
end

% Assume there isn't anyone outside
im=~rays.activemask;

% Draw all visible rays
if doplot>1
  cnt0=zeros(size(im));
  cnt1=zeros(size(im));
end
for c=1:size(v,1)
  v1=find(v(c,:)==1);
  % OLD way of doing this; was too slow
  % isone=ismember(rays.rays{c},v1,'R2012a');
  visone=v(c,:)==1;
  im(rays.nzrayindices{c}(visone(rays.nzraymap{c})))=1;
  if doplot>1
    isone=ismember(rays.rays{c},v1,'R2012a');
    cnt1(isone)=cnt1(isone)+1;
    %  fprintf('c=%d, cnt0(500,500)=%d, cnt1=%d\n', c, cnt0(500,500), cnt1(500,500));
    % Count rays blocked by something inside (ie not including ones that high LEDs in corridor, since something there could be blocking them)
    iszero=ismember(rays.rays{c},find(v(c,:)==0 & ~layout.outsider'),'R2012a');
    cnt0(iszero)=cnt0(iszero)+1;
  end
end

% Convert target diameters into (float) pixel coords
mintgtdiampix=m2pix(rays.imap,p.analysisparams.mintgtdiam);
maxfalsegap=m2pix(rays.imap,p.analysisparams.maxfalsegap);

% Erode image using disk
% structure element slightly smaller than a person
shrink=floor(mintgtdiampix/2);
se=strel('disk',shrink,0);
possible=imerode(im==0,se);	% Erode image (1==center of person could be here)

% Regrow the region 
ibd=imdilate(possible,se);	% Regrow it (1==person could be present)

% Dilate image using disk to close any gaps that may be between legs
sefg=strel('disk',ceil(maxfalsegap/2),0);   %Despite documentation, using N=0 is more than twice as fast than N=4
rmgaps1=imdilate(ibd,sefg);
rmgaps=imerode(rmgaps1,sefg);  % dilate then erode

lbl=possible.*bwlabel(rmgaps);		% Label gap removed image, then mask with eroded image (only pixels that could be centers)
                                        % This will prevent a narrowing between the legs from separating them into 2 separate labels

% The masking above could have completely removed some labels, watch out...

% Figure out centers of each possible target
% Use modified version of regionprops() to avoid time spent on input sanity checking
stats=fastregionprops(lbl','Centroid','Area','MinorAxisLength','MajorAxisLength','Orientation','PixelList');  % Use transpose to go back to coord axis

% Create structure of results
tgts=struct('pos',{},'minoraxislength',{},'majoraxislength',{},'orientation',{},'convexhull',{},'area',{},'stats',{},'nunique',{},'pixellist',{});
for i=1:length(stats)
  pos=pix2m(rays.imap,stats(i).Centroid);
  minoraxislength=(stats(i).MinorAxisLength+2*shrink)/rays.imap.scale;
  majoraxislength=(stats(i).MajorAxisLength+2*shrink)/rays.imap.scale;
  % convexhull=pix2m(rays.imap,stats(i).ConvexHull);
  convexhull=nan;
  pixellist=single(pix2m(rays.imap,stats(i).PixelList));
  area=((sqrt(stats(i).Area)+2*shrink)/rays.imap.scale)^2;
  % Due to the masking, regionprops sometimes returns an empty region (no pixels)
  if isempty(pixellist) 
    fprintf('Empty label index %d\n',i);
  end
  tgts=[tgts,struct('pos',pos,'minoraxislength',minoraxislength,'majoraxislength',majoraxislength,'orientation',stats(i).Orientation,'convexhull',convexhull,'pixellist',pixellist,'area',area,'stats',stats(i),'nunique',nan)];
end
        
if doplot>1
  imc=zeros(size(im,1),size(im,2),4);
  imc(:,:,1)=cnt0/size(v,1);
  imc(:,:,2)=cnt1/size(v,1);
  imc(:,:,3)=rmgaps;
  
  setfig('analyze.rays');clf;
  for k=1:2
    subplot(1,2,k);
    hold on;
    if k==1
      imshowmapped(rays.imap,imc(:,:,[1 4 3]).*repmat(rays.activemask,[1,1,3]));
    else
      imshowmapped(rays.imap,imc(:,:,[4 2 3]).*repmat(rays.activemask,[1,1,3]));
    end
    if k==1
      title('Blocked rays');
    else
      title('Visible rays');
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
  end
end

% Compute which points are possible ghosts
if length(tgts)>0
  alllbl=bwlabel(rmgaps==1);	% Label regrown image
  %assert(length(unique(alllbl))==length(tgts)+1);  % This assert slows things down a lot...

  for i=1:length(tgts)
    tgts(i).nunique=0;
  end
  [cind,lind]=ind2sub(size(v),find(v(:)==0));
  for i=1:length(cind)
    c=cind(i);
    l=lind(i);
    if ~isempty(layout.outsider) && layout.outsider(l)
      % fprintf('Skipping outsider %d\n', l);
      continue;
    end
    %      profile=improfile(lbl,rays.raylines{c,l}(:,2),rays.raylines{c,l}(:,1));
    % Count distinct objects on each ray
    profile=alllbl(fastsub2ind(size(lbl),rays.raylines{c,l}(:,1),rays.raylines{c,l}(:,2)));
    profile=profile(profile~=0);
    if ~isempty(profile) && all(profile==profile(1))
      % Unique object
      if profile(1)>length(tgts)
        % TODO: Bug hit at least once here, profile(1)==9, but tgts is 1x8 
        % Relabelling above might be giving different number of targets
        % Would also affect cases where the ordering changes, but wouldn't be detected here
        fprintf('ERROR: profile=%d, but length(tgts)=%d\n', profile(1),length(tgts));
      else
        tgts(profile(1)).nunique=tgts(profile(1)).nunique+1;
      end
    end
  end
end

% Remove any empty targets (due to label masking above)
keeptgt=true(1,length(tgts));
for i=1:length(tgts)
  if isempty(tgts(i).pixellist)
    keeptgt(i)=false;
  end
end
tgts=tgts(keeptgt);

if doplot>0
  setfig('analyze.estimates');
  clf;
  imshowmapped(rays.imap,(1-ibd/2));
  title('analyze.estimates');
  hold on;
  if exist('truetgts','var')
    viscircles(truetgts.tpos,truetgts.tgtdiam/2,'LineWidth',0.5);
  end
  for i=1:length(tgts)
    cent=tgts(i).pos;
    plot(cent(1),cent(2),'or');
    % plot(tgts(i).convexhull(:,1),tgts(i).convexhull(:,2),'g');
    text(cent(1),cent(2),sprintf('%d (%d)',i,tgts(i).nunique),'color','k','HorizontalAlignment','center');
    fprintf('Object %d at (%.1f,%.1f) was unique for %d rays\n', i, cent, tgts(i).nunique);
  end
end

snap=struct('tgts',tgts);

function ndx=fastsub2ind(siz,v1,v2)
ndx=v1+(v2-1).*siz(1);
% getvisible - see what LEDs are visible to the cameras
% Usage:  vis=getvisible(p,options)
%	  [vis,p]=getvisible(p,'init',true);
% Options:
%	setleds:  true to turn leds on/off (default true)
%	im:	  image set to use instead of acquisition from cameras (default {})
%	stats:	  true to return full statisics in vis struct (default false)
%	calccorr:	  true to calculate correlations (default true)
%	mincorr:  threshold old value for correlation to indicate LED is unblocked (default: 0.5)
%	onval:	  3x1 vector holding RGB values of LED to use (defaults to p.colors{1})
%	init:	  true to initialize data for operation, store in returned p struct (default false)
%	usefrontend: use frontend to acquire high-speed stream (default true)
% When 'init' is used, additional options are available:
%	wsize:    2x1 size of window, in pixels, which will be centered on centroid of LED (default: [11 11], for init only)
%	navg:	  number of samples to average over (default=number of colors in p.colors; which are used as onval)
% Returns vis with fields:
% 	v(ncam,nled) - 1 if LED is visible, 0 if not
%	when - when acquisition too place
%	corr - correlation with template
% when 'init' is used, the returned 'vis' is a cell array of 'navg' normal vis structs, each with full stats, except correlations
% If 'stats' option is passed in, also sets:
% 	lev(ncam,nled) - level at each LED spot
% 	im{ncam} - full images
%	tgt{ncam,nled} - images of each target
function [vis,p]=getvisible(p,varargin)
args=struct('setleds',true,'im',{{}},'stats',false,'init',false,'onval',127*p.colors{1},'wsize',[11 11],'navg',2*length(p.colors),'calccorr',true,'mincorr',0.5,...
            'usefrontend',true);
i=1;
while i<=length(varargin)
  if ~isfield(args,varargin{i})
    error('Unknown option: "%s"\n',varargin{i});
  end
  args.(['SET',varargin{i}])=true;   % Flag that it was set explicitly
  if islogical(args.(varargin{i})) && (i==length(varargin) || ischar(varargin{i+1}))
    % Special case of option string without value for boolean, assume true
    args.(varargin{i})=true;
  else
    args.(varargin{i})=varargin{i+1};
    i=i+1;
  end
  i=i+1;
end

if args.init
  if nargout~=2
    error('Usage:  [vis,p]=getvisible(p,''init'')');
  end
else
  if nargout~=1
    error('Usage:  vis=getvisible(p, options)');
  end
  if ~isfield(p.camera(1),'viscache')
    error('"viscache" data structure not initialized; use "init" option first');
  end
end

if args.init
  % Initialize data in p.camera(:).viscache needed by getvisible
  fprintf('Initializing viscache...\n');
  for i=1:length(p.camera)
    c=p.camera(i).pixcalib;
    roi=p.camera(i).roi;
    % Valid LED maps
    fvalid=find([c.valid]);
    % Setup indirect access to data
    ind=[];
    tlpos=nan(length(p.led),2);
    brpos=tlpos;
    imgsize=[roi(4)-roi(3),roi(2)-roi(1)];
    for vj=1:length(fvalid)
      j=fvalid(vj);
      tpos=c(j).pos-[roi(1),roi(3)]+1;
      [s1,s2]=ind2sub(args.wsize,1:prod(args.wsize));
      s1=ceil(tpos(:,2)+s1-mean(s1));
      s2=ceil(tpos(:,1)+s2-mean(s2));
      % Shift window so it falls inside image
      if min(s1)<1
        fprintf('Shifting window down by %d pixels\n',1-min(s1));
        s1=s1-min(s1)+1;
      end
      if min(s2)<1
        fprintf('Shifting window right by %d pixels\n',1-min(s2));
        s2=s2-min(s2)+1;
      end
      if max(s1)>imgsize(1)
        fprintf('Shifting window up by %d pixels\n',max(s1)-imgsize(1));
        s1=s1-max(s1)+imgsize(1);
      end
      if max(s2)>imgsize(2)
        fprintf('Shifting window left by %d pixels\n',max(s2)-imgsize(2));
        s2=s2-max(s2)+imgsize(2);
      end

      ind(vj,:)=sub2ind(imgsize,s1,s2);
      tlpos(j,:)=[s2(1),s1(1)];
      brpos(j,:)=[s2(end),s1(end)];
    end
    p.camera(i).viscache=struct('wsize',args.wsize,'imgsize',imgsize,'indices',ind,'when',now,'tlpos',tlpos,'brpos',brpos);
  end

  % Acquire samples
  for i=1:args.navg
    if isfield(args,'SETonval')
      if iscell(args.onval)
        if length(args.onval)~=args.navg
          error('Initialized with %d samples, but passed in array of %d onvals\n',args.navg,length(args.onval));
        end
        onval{i}=args.onval{i};   % Can pass in cell array of onvals to use these for calibration
      else
        onval{i}=args.onval;
      end
    else
      % Use pix colors
      onval{i}=p.colors{mod(i-1,length(p.colors))+1}*127;
    end
    % Recursive call without init, full stats, no corr (since viscache not setup)
    vis{i}=getvisible(p,'calccorr',false,'stats',true,'onval',onval{i});
  end

  % Take average to set template (in ref{} as individual images, and in refvec(l,:) as a matrix of vectors, 1 row per LED
  for c=1:length(p.camera)
    % Compute total reference image
    refim=single(vis{1}.im{c});
    for j=2:length(vis)
      refim=refim+single(vis{j}.im{c});
    end
    refim=refim/length(vis)/255;
    p.camera(c).viscache.refim=refim;
    
    pc=p.camera(c).pixcalib;
    p.camera(c).viscache.refimspot=cell(1,length(p.led));
    p.camera(c).viscache.inuse=[pc.valid];   % Initially same as valid (may be changed below)
    fvalid=find([pc.valid]);
    p.camera(c).viscache.ledmap=fvalid;
    sc=nan(length(p.led),length(vis));
    for j=1:length(fvalid)
      l=fvalid(j);
      s=single(vis{1}.imspot{c,l});
      for i=2:length(vis)
        s=s+single(vis{i}.imspot{c,l});
      end
      s=s/length(vis);
      p.camera(c).viscache.refimspot{l}=uint8(round(s));
      s=s-mean(s(:));s=s/std(s(:));

      p.camera(c).viscache.refvec(j,:)=s(:);
    end
  end
  % Check correlation of each image with avg
  for i=1:length(vis)
    vistmp=getvisible(p,'im',vis{i}.im,'stats',true);
    for c=1:length(p.camera)
      p.camera(c).viscache.refcorr(:,i)=vistmp.corr(c,:);
    end
  end

  % Disable LEDs that show low correlation
  for c=1:length(p.camera)
    for l=1:length(p.led)
      if p.camera(c).pixcalib(l).valid
        [mincorr,ord]=min(p.camera(c).viscache.refcorr(l,:));
        if mincorr < 0.7
          fprintf('Warning: disabling pixel due to min corr <0.7; corr(%d,%d)=%.4f with [%d,%d,%d]\n', c,l,mincorr, onval{ord});
          p.camera(c).viscache.inuse(l)=false;
        end
      end
    end
  end
  
  % Done with init
  fprintf('Done initializing viscache...\n');
  % In the init case, the returned vis is a cell array of args.navg vis structs
  if args.usefrontend
    fprintf('Sending initialize data to front end\n');
    rcvr(p,'init');
  end
  return;
end

% Normal case -- not init
if ~isempty(args.im)
  args.setleds=false;
end

if args.setleds && ~args.init
  s1=arduino_ip();
  % Turn on all LED's
  %  fprintf('Turning on LEDs\n');
  setled(s1,-1,args.onval,1);
  show(s1);
  sync(s1);
  % even sending second sync does not ensure that the strip has been set
  % pause for 300ms (200ms sometimes wasn't long enough)
  pause(0.3);
end

if args.usefrontend  && ~args.stats
  while true
    % Call frontend to get next frame
    vis=rcvr(p);
    vis.whenrcvd=now;
    age=(vis.whenrcvd-max(vis.acquired))*24*3600;
    if (age>2)
      fprintf('Skipping stale frame %d that is %.1f seconds old\n', vis.frame(1), age);
    else
      fprintf('Got frame %d that is %.3f seconds old\n',vis.frame(1),age);
      return;
    end
  end
end

% Initialize result struct
vis=struct('when',now);

if ~isempty(args.im)
  im=args.im;
else
  im=aremulti([p.camera.id],p.camera(1).type,{p.camera.roi});
end

% Convert to gray if needed, check size
imorig=im;
for i=1:length(im)
  vc=p.camera(i).viscache;
  if length(vc.imgsize)==2 && length(size(im{i}))==3
    % Convert to gray since window was setup for gray scale image
    % Use ratios to allow bitshift operation, more weight for red
    im{i}=rgb2graywithweight(im{i});
  end
  if any(vc.imgsize ~= size(im{i}))
    error('Incorrect image size from camera %d: expected [%s], read [%s]\n', i, sprintf('%d ',vc.imgsize),sprintf('%d ',size(im{i})));
  end
end

if args.setleds
  % Turn off LEDs
  setled(s1,-1,[0,0,0]);
  show(s1);
end

if args.stats
  vis.imspot={};
  vis.lev=[];
  vis.im=imorig;   % Original image, might have been converted to gray for processing
  for i=1:length(im)
    c=p.camera(i).pixcalib;
    vc=p.camera(i).viscache;
    if isfield(vc,'refim')
      vis.refim{i}=vc.refim;
    end
    
    % Valid LED maps
    fvalid=find([c.valid]);

    % Linear indices into images organized by LEDs listed in ledmap (rows), and window position (col)
    ind=vc.indices;

    % Size of individuals windows in screen coords
    wsize=p.camera(i).viscache.wsize;
    assert(size(ind,2)==prod(wsize));

    refvec=[];

    % Pack all the LEDs into matrix (to speed up correlation computation)
    w=reshape(im{i}(ind(:)),size(ind,1),size(ind,2));
    for vj=1:length(fvalid)
      j=fvalid(vj);
      vis.imspot{i,fvalid(vj)}=reshape(w(vj,:),wsize);
      % Compute level as max over pixcalib.pixelList (using cached linear indices)
      if size(im{i},3)==3
        vis.lev(i,j)=max(im{i}(c(j).rgbindices));
      else
        vis.lev(i,j)=max(im{i}(c(j).indices));
      end
    end
  end
end

if args.calccorr
  vis.corr=nan(length(p.camera),length(p.camera(1).pixcalib));
  
  for i=1:length(im)
    vc=p.camera(i).viscache;

    % Linear indices into images organized by LEDs listed in ledmap (rows), and window position (col)
    ind=vc.indices;
    
    oldway=0;
    if oldway
      % Compute cross-correlation with ref image
      % Pack all the LEDs into matrix (to speed up correlation computation) using linear index in vc.indices
      w=single(reshape(im{i}(ind(:)),size(ind,1),size(ind,2)));
      % Calc cross-correlation
      sww=sum(w.*w,2)+0.1;   % Add a little fuzz in case the image is a constant value, which would otherwise result in a NaN for the correlation
      sw=sum(w,2);
      swr=sum(w.*vc.refvec,2);
      vis.corr(i,vc.ledmap)=swr./sqrt(sww*size(ind,2)-sw.*sw);
    else
      % Same way as frontend, max(corr) with each color plan
      corrplane=[];
      N=size(ind,2);
      for plane=1:3
        % Compute cross-correlation with ref image
        % Pack all the LEDs into matrix (to speed up correlation computation) using linear index in vc.indices
        implane=imorig{i}(:,:,plane);
        refimplane=vc.refim(:,:,plane);
        w=single(reshape(implane(ind(:)),size(ind,1),size(ind,2)));
        r=single(reshape(refimplane(ind(:)),size(ind,1),size(ind,2)));
        % Calc cross-correlation
        sww=sum(w.*w,2);
        sw=sum(w,2);
        swr=sum(w.*r,2);
        srr=sum(r.*r,2);
        sr=sum(r,2);
        denom=sqrt((sww-sw.*sw/N).*(srr-sr.*sr/N));
        corrplane(plane,:)=(swr-sw.*sr/N)./denom;
      end
      vis.corr(i,vc.ledmap)=max(corrplane,[],1);
    end
  end

  
  vis.mincorr=args.mincorr;
  vis.v=single(vis.corr>vis.mincorr);
  vis.v(isnan(vis.corr))=nan;

  % Turn off indicator for pixels not in use (but lev, corr still valid)
  for c=1:length(p.camera)
    % DEBUGGING situation where corr=NaN due to constant value image (see fuzz above)
    %if any(isnan(vis.corr(c,[p.camera(c).viscache.inuse])))
    %  fprintf('Bad corr.');
    %  keyboard;
    %end
    vis.v(c,~[p.camera(c).viscache.inuse])=nan;
  end
end
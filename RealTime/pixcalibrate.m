% Run pixel calibration to determine which pixels of each camera map to which LED
% Usage: [sainfo,im]=pixcalibrate(sainfo,im)
% sainfo - structure holding configuration/calibration information
% im - optional arg to reuse image data
function [sainfo,im]=pixcalibrate(sainfo,im)
ids=[sainfo.camera.id];
DEBUG=true;
nrpt=1;
nled=numled();
nbits=ceil(log2(nled));
minpixels=1;   % Minimum number of active pixels to accept an LED
if nargin<2 || isempty(im)
  lsctl(sainfo,'pause');
  s1=arduino_ip();
  tic;
  nset=0;
  level=127;
  fprintf('At start Arduino already received %d bytes\n', getnumrcvd(s1));
  nsent=zeros(1,nrpt);
  nrcvd=zeros(1,nrpt);

  % Get image with LEDs off
  imtmp=aremulti(ids,sainfo.camera(1).type);

  for i=1:length(imtmp)
    % Check for clipping
    nclip=sum(imtmp{i}(:)>252);
    if nclip>0
      fprintf('Image from camera %d clipped at %d pixels before any LEDs turned on\n', ids(i), nclip);
    end
  end

  im=cell(nbits,2,nrpt,length(ids));

  for p=1:nrpt
    nsent(p)=0;
    for k=uint32(1):nbits
      for j=1:2
        c=[];
        pos=1;
        for i=uint32(0):nled-1
          if bitget(i,k)==j-1
            cmd=setled(s1,i,level*[1,1,1],0);
          else
            cmd=setled(s1,i,level*[0,0,0],0);
          end
          c(pos:pos+length(cmd)-1)=cmd;
          pos=pos+length(cmd);
        end
        awrite(s1,c);
        nsent(p)=nsent(p)+length(c);
        show(s1);
        nsent(p)=nsent(p)+1;
        sync(s1);
        nsent(p)=nsent(p)+2;  % For sync()
                              % Wait at least one frame time
        pause(1);
        % Get image
        imtmp=aremulti(ids,sainfo.camera(1).type);
        for ii=1:length(imtmp)
          im{k,j,p,ii}=imtmp{ii};
        end
%        for iid=1:length(ids)
%          id=ids(iid);
%          imtmp=arecont(id);
%          im{k,j,p,iid}=imtmp.im;
%        end
      end
      nset=nset+1;
    end
    % Now clear them all
    c=setled(s1,-1,[0,0,0],0);
    awrite(s1,c);
    nsent(p)=nsent(p)+length(c);
    show(s1);
    sync(s1);
    nsent(p)=nsent(p)+3;  % For show+sync

    nrcvd(p)=getnumrcvd(s1);
    nsent(p)=nsent(p)+1;  % For getnumrcvd() cmd
%%    fprintf('Iteration %d, sent %d, Arduino received %d\n', p, nsent(p), nrcvd(p));
    if nsent(p)~=nrcvd(p)
      fprintf('**** Dropped %d bytes (Send %d, Arduino received %d)\n', nsent(p)-nrcvd(p),nsent(p),nrcvd(p));
    end
  end
  %  elapsed=toc;
  %  fprintf('Changed all %d LEDs %d times in %.2f seconds = %.2f changes/second\n', nled, nset, elapsed, nset/elapsed);
  %  fprintf('Wrote %d bytes, Read %d bytes in %.2f seconds. Effective rate = %.0f bytes/second\n', stopwrote-startwrote, stopread-startread, elapsed, (stopwrote-startwrote+stopread-startread)/(elapsed));
end


for iid=1:length(ids)
  id=ids(iid);
  fprintf('\n*** Processing data for camera %d\n',id);
  if DEBUG
    setfig(sprintf('pixcompare%d',id));
    clf;
  end
  % Form imat which indexes each pixel with the LED number there
  % Initial will contain 0..nled-1
  imat=[];
  % In parallel, form tmat, which indicates where signal is strong enough to id a pixel
  se=strel('disk',2,0);
  % Loop over the bits of the LED numbers
  z=cell(nbits,2);
  for k=1:nbits
    for j=1:2
      z{k,j}=rgb2gray(im2single(im{k,j,1,iid}));
      for p=2:size(im,3)
        z{k,j}=z{k,j}+rgb2gray(im2single(im{k,j,p,iid}));
      end
      z{k,j}=z{k,j}/size(im,3);
      if DEBUG
        subplot(nbits,3,k*3+j-3);
        imshow(z{k,j});
      end
    end
    zdiff=z{k,2}-z{k,1};
    zd{k}=zdiff;
    % bw is where this particular bit is more likely on than off
    bw=zd{k}>0;

    if k==1
      imat=(bw)*2^(k-1);
      tmat=abs(zd{k});
    else
      imat=(bw)*2^(k-1)+imat;
      tmat=min(tmat,abs(zd{k}));
    end
    if DEBUG
      subplot(nbits,3,k*3)
      imshow(zdiff/max(zdiff(:))+0.5);
      pause(0);
    end
  end
  if DEBUG
    % Add main title
    suptitle(sprintf('Camera %d images',id));
  end
  % Change indexing to 1..nled (so we can 0 to mean unlabelled)
  imat=(imat+1);
  % Only label points that have a minimum signal value
  tmatthresh=max(tmat(:))/6;
  selmat=tmat>=tmatthresh;
  fprintf('Thresh(%d)=%f with %d points>thresh\n',iid, tmatthresh, sum(selmat(:)));

  % Unlabel invalid points
  imat(~selmat)=0;
  if DEBUG
    % Plot count of pixels per led
    pcnt=zeros(1,nled);
    for i=1:nled
      pcnt(i)=sum(sum(imat==i));
    end
    setfig(sprintf('pixcnt%d',id));
    plot(pcnt,'o');
    xlabel('LED Number');
    ylabel('Pixels Covered');
    title(sprintf('Pixels Covered/LED for camera %d (Median=%.1f)',id,median(pcnt)));
  end

  % Unlabel points that refer to non-existent LED's
  imat(~ismember(imat,[sainfo.led.id]))=0;
  % Display as an RGB image
  if DEBUG
    setfig(sprintf('indexmat%d',id));
    imshow(label2rgb(imat));
    title(sprintf('Index image for camera %d',id));
  end

  % Break each distinct LED into regions
  xany=any(imat,1);
  yany=any(imat,2);
  roi=[find(xany,1),find(xany,1,'last'),find(yany,1),find(yany,1,'last')];
  imatroi=imat(roi(3):roi(4),roi(1):roi(2));
  fprintf('Size(imatroi(%d))=%d %d; roi=%d %d %d %d\n', iid,size(imatroi),roi);
  calib=struct([]);
  for i=1:length(sainfo.led)
    ledid=sainfo.led(i).id;
    lpos=imatroi==ledid;
    stats=regionprops(bwconncomp(lpos),'all');
    for j=1:length(stats)
      stats(j).Centroid=stats(j).Centroid+roi([1,3])-1;
    end
    calib(i).stats=stats;
    if isempty(stats)
      calib(i).pos=[nan,nan];
      calib(i).diameter=nan;
      calib(i).valid=false;
    else
      [maxarea,mpos]=max([stats.Area]);
      if size(stats(mpos).PixelList,1)<minpixels
        calib(i).valid=false;
        fprintf('LED %3d is only visible to camera %d at %d pixels; ignoring.\n',...
                ledid,id,size(stats(mpos).PixelList,1));
      else
        calib(i).valid=true;
        minarea=maxarea;
        for j=1:length(stats)
          if j~=mpos && stats(j).Area >= minarea
            fprintf('LED %3d is also visible to camera %d at (%4.0f,%4.0f) in addition to (%4.0f,%4.0f), distance=%.1f\n',...
                    ledid,id,stats(j).Centroid,stats(mpos).Centroid,norm(stats(j).Centroid-stats(mpos).Centroid));
            % TODO - if close, could merge with first region
          end
        end
      end
      calib(i).pos=stats(mpos).Centroid;
      calib(i).diameter=stats(mpos).EquivDiameter;
      calib(i).pixelList=stats(mpos).PixelList;
      % Compute indices of pixels in cropped images
      calib(i).indices=sub2ind(size(imatroi),calib(i).pixelList(:,2),calib(i).pixelList(:,1));
      % Pixel list in full imagespace coords
      calib(i).pixelList(:,1)=calib(i).pixelList(:,1)+roi(1)-1;
      calib(i).pixelList(:,2)=calib(i).pixelList(:,2)+roi(3)-1;
    end
  end
  % Check for stray points
  maxpixelsep=50;
  for i=1:length(sainfo.led)
    if calib(i).valid 
      closest=2e10;
      nledsep=0;
      for j=i-1:-1:1
        if calib(j).valid
          closest=norm(calib(i).pos-calib(j).pos);
          nledsep=i-j;
          break;
        end
      end
      for j=i+1:length(sainfo.led)
        if calib(j).valid
          closest=min(closest,norm(calib(i).pos-calib(j).pos));
          nledsep=min(nledsep,j-i);
          break;
        end
      end

      if nledsep>0 && closest>maxpixelsep*nledsep
        fprintf('LED %d is at least %d pixels from its neighbor (%d LEDs away) -- rejecting it\n',i,closest, nledsep);
        calib(i).valid=false;  
      end
      calib(i).closest=closest;
    end
  end

  % Check for Camera->LED that don't fall inside active area by at least 5% of the radius to the midpoint
  for i=1:length(sainfo.led)
    if calib(i).valid
      midpoint=(sainfo.layout.cpos(iid,:)+sainfo.layout.lpos(i,:))/2;
      if ~inpolygon(midpoint(1)*1.05,midpoint(2)*1.05,sainfo.layout.active(:,1),sainfo.layout.active(:,2))
        fprintf('LED %d to camera midpoint at (%.1f,%.1f) is not far enough inside active area -- rejecting it\n', i, midpoint);
        calib(i).valid=false;
      end
    end
  end
  

  for i=1:length(sainfo.led)
    calib(i).inuse=calib(i).valid;   % For now, all valid pixels can be used
  end
  
  notvis=find(~[calib.valid]);
  if ~isempty(notvis)
    fprintf('LEDs not visible to camera %d: %s\n', id, shortlist(notvis));
  end

  sainfo.camera(iid).pixcalib=calib;
  sainfo.camera(iid).pixcalibtime=now;
  % Setup ROI for each camera
  cp=reshape([calib([calib.valid]).pos],2,[]);
  border=min(6,max([calib([calib.valid]).diameter]));
  sainfo.camera(iid).roi=[
      max(1,floor(min(cp(1,:))-border)),min(size(imat,2),ceil(max(cp(1,:))+border)),...
      max(1,floor(min(cp(2,:))-border)),min(size(imat,1),ceil(max(cp(2,:))+border))];
  % Make divisible by 32 for camera
  sainfo.camera(iid).roi([1,3])=floor((sainfo.camera(iid).roi([1,3])-1)/32)*32+1;
  sainfo.camera(iid).roi([2,4])=ceil((sainfo.camera(iid).roi([2,4])-1)/32)*32+1;
  roi=sainfo.camera(iid).roi;
  fprintf('Camera %d: ROI size = %d x %d\n', iid, (roi(2)-roi(1)),(roi(4)-roi(3)));
  if (roi(4)-roi(3))>200
    fprintf('**WARNING** Camera %d has excessive ROI size = %d x %d\n', iid, (roi(2)-roi(1)),(roi(4)-roi(3)));
  end
end % iid

% Setup indices from pixelList
for iid=1:length(sainfo.camera)
  roi=sainfo.camera(iid).roi;
  for i=1:length(sainfo.camera(iid).pixcalib)
    if sainfo.camera(iid).pixcalib(i).valid
      pixelList=sainfo.camera(iid).pixcalib(i).pixelList;
      pixelList(:,1)=pixelList(:,1)-roi(1)+1;
      pixelList(:,2)=pixelList(:,2)-roi(3)+1;
      % Setup indices from pixellists
      sainfo.camera(iid).pixcalib(i).indices=...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1)],pixelList(:,2),pixelList(:,1));
      sainfo.camera(iid).pixcalib(i).rgbindices=[...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1),3],pixelList(:,2),pixelList(:,1),1*ones(size(pixelList,1),1));...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1),3],pixelList(:,2),pixelList(:,1),2*ones(size(pixelList,1),1));...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1),3],pixelList(:,2),pixelList(:,1),3*ones(size(pixelList,1),1))];
    end
  end
end

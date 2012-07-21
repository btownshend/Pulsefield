% getvisible - see what LEDs are visible to the cameras
% Returns vis with fields:
% v(ncam,nled) - 1 if LED is visible, 0 if not
% lev(ncam,nled) - level at each LED spot
% im(ncam) - images
function vis=getvisible(sainfo,setleds,onval)
if nargin<2
  setleds=true;
end
if nargin<3
  onval=sainfo.colors{1};
end
if setleds
  s1=arduino_ip();
  % Turn on all LED's
  %  fprintf('Turning on LEDs\n');
  setled(s1,-1,onval,1);
  show(s1);
  sync(s1);
  % even sending second sync does not ensure that the strip has been set
  % pause for 300ms (200ms sometimes wasn't long enough)
  pause(0.3);
end
rois={sainfo.camera.roi};
when=now;
im=aremulti([sainfo.camera.id],sainfo.camera(1).type,rois);
if setleds
  % Turn off LEDs
  setled(s1,-1,[0,0,0]);
  show(s1);
end
if strcmp(sainfo.analysisparams.visalgorithm,'maxlev')
  lev=nan(length(sainfo.camera),length(sainfo.camera(1).pixcalib));
  for i=1:length(im)
    c=sainfo.camera(i).pixcalib;
    % Valid LED maps
    fvalid=find([c.valid]);
    for vj=1:length(fvalid)
      j=fvalid(vj);
      % Maximum pixel value for camera i, led j
      lev(i,j)=max(im{i}(c(j).rgbindices));
    end
  end
  if isfield(sainfo,'crosstalk')
    v=single(lev>sainfo.crosstalk.thresh);
    v(isnan(lev))=nan;
  elseif strcmp(sainfo.analysisparams.visalgorithm,'maxlev')
    % Just assume a threshold level
    fprintf('Warning: should run crosstalk to get better thresholds\n');
    v=single(lev>sainfo.analysisparams.thresh);
  end
  vis=struct('v',v,'lev',lev,'im',{im},'when',when);
elseif strcmp(sainfo.analysisparams.visalgorithm,'xcorr')
  xcorr=nan(length(sainfo.camera),length(sainfo.camera(1).pixcalib));
  bbwind=sainfo.analysisparams.bbwind;
  wsize=(bbwind*2+1)^2;
  for i=1:length(im)
    c=sainfo.camera(i).pixcalib;
    roi=sainfo.camera(i).roi;
    % Valid LED maps
    fvalid=find([c.valid]);
    for vj=1:length(fvalid)
      j=fvalid(vj);
      % Maximum pixel value for camera i, led j
      %lev(i,j)=max(im{i}(c(j).rgbindices));  % Can compute for crosstalk use, but need to comment out when running for speed

      % Compute cross-correlation with ref image
      tpos=ceil(c(j).pos-[roi(1),roi(3)]);
      w=double(im{i}(tpos(:,2)+(-bbwind:bbwind),tpos(:,1)+(-bbwind:bbwind)));
      % Normalize
      w=w(:)-sum(w(:))/wsize;
      % Correlation between tgt and ref
      xcorr(i,j)=w'*c(j).ref(:);
      xcorr(i,j)=xcorr(i,j)/sqrt((w'*w)/wsize);
      tgt{i,j}=w;
    end
  end
  v=single(xcorr>sainfo.analysisparams.mincorr);
  v(isnan(xcorr))=nan;
%  vis=struct('v',v,'lev',lev,'xcorr',xcorr,'im',{im},'tgt',{tgt},'when',when);
  vis=struct('v',v,'xcorr',xcorr,'im',{im},'tgt',{tgt},'when',when);
else
  error('Unimplemented vis algorithm: %s\n',sainfo.analysisparams.visalgorithm);
end
% Turn off indicator for pixels not in use (but lev, xcorr still valid)
for c=1:length(sainfo.camera)
  vis.v(c,~[sainfo.camera(c).pixcalib.inuse])=nan;
end

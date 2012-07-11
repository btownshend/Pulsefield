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
  fprintf('Turning on LEDs\n');
  setled(s1,-1,onval,1);
  show(s1);
  sync(s1);
  % even sending second sync does not ensure that the strip has been set
  % pause for 300ms (200ms sometimes wasn't long enough)
  pause(0.3);
end
rois={sainfo.camera.roi};
when=now;
im=aremulti([sainfo.camera.id],rois);
if setleds
  % Turn off LEDs
  setled(s1,-1,[0,0,0]);
  show(s1);
end
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
  % Turn off indicators if margin was too low
  v(sainfo.crosstalk.margin<sainfo.analysisparams.minmargin)=nan;
else
  % Just assume a threshold level
  v=single(lev>p.analysisparams.thresh);
end
vis=struct('v',v,'lev',lev,'im',{im},'when',when);

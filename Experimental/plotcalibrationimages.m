function plotcalibrationimages(p,vis,varargin)
defaults=struct('addpoints',[]);
args=processargs(defaults,varargin);

roomheight=3;   % Height of room in meters
step=0.05;      % Step size for drawing active area outline in meters
setfig('calibimages');clf;
fprintf('G=visible, R=blocked, Y=disabled(intermediate), M=disabled(high variance), Cyan=disabled(unused)\n');
for c=1:length(p.camera)
  cam=p.camera(c);
  scale=cam.hpixels/cam.distortion.Wcal;
  subplot(2,3,c);
  hold on;
  if nargin>=2 && isfield(vis,'im')
    imtmp=imresize(cam.extcal.image.im,scale)/2;
    imtmp(cam.roi(3):cam.roi(4)-1,cam.roi(1):cam.roi(2)-1,:)=vis.im{c};
    imshow(imtmp);
  else
    imshow(imresize(cam.extcal.image.im,scale));
  end
  pcy=arrayfun(@(z) z.pos(2),cam.pixcalib);
  pcx=arrayfun(@(z) z.pos(1),cam.pixcalib);
  if nargin<2
    plot(pcx,pcy,'.');
  else
    % Colorcode visible LEDs
    if isfield(vis,'vorig')
      v=vis.vorig(c,:);
    else
      v=vis.v(c,:);
      v(isnan(v))=2;
    end
    v(~cam.viscache.inuse)=4;
    sel=v==0; plot(pcx(sel),pcy(sel),'.r');
    sel=v==1; plot(pcx(sel),pcy(sel),'.g');
    sel=v==2; plot(pcx(sel),pcy(sel),'.y');
    sel=v==3; plot(pcx(sel),pcy(sel),'.m');
    sel=v==4; plot(pcx(sel),pcy(sel),'.c');
  end
  % Plot all the other cameras
  for c2=1:length(p.camera)
    if c2==c
      continue;
    end
    cpos=[p.layout.cpos(c2,:),p.layout.cposz(c2)];
    pixelpos=getpixelpos(cam,cpos);
    if ~any(isnan(pixelpos))
      plot(pixelpos(1),pixelpos(2),'xr');
    end
  end
  % Plot the room active area
  pathlen=cumsum([0;sqrt(diff(p.layout.active(:,1)).^2+diff(p.layout.active(:,2)).^2)]);
  a=[];
  a(:,1)=interp1(pathlen,p.layout.active(:,1),0:step:pathlen(end));
  a(:,2)=interp1(pathlen,p.layout.active(:,2),0:step:pathlen(end));
  % Draw lines along floor and ceiling 
  p0=[];
  ph=[];
  for i=1:size(a,1)
    p0(i,:)=getpixelpos(cam,[a(i,:),0]);
    ph(i,:)=getpixelpos(cam,[a(i,:),roomheight]);
  end
  plot(p0(:,1),p0(:,2),'c');
  plot(ph(:,1),ph(:,2),'c');
  % Draw corner verticals
  for i=1:size(p.layout.active,1)
    pp=getpixelpos(cam,[p.layout.active(i,:),0]);
    for h=0:step:roomheight;
      lastpp=pp;
      pp=getpixelpos(cam,[p.layout.active(i,:),h]);
      if ~any(isnan([lastpp,pp]))
        plot([lastpp(1),pp(1)],[lastpp(2),pp(2)],'c');
      end
    end
  end
  % Add any points given on command line
  for i=1:size(args.addpoints,1)
    pp=getpixelpos(cam,args.addpoints(i,:));
    if ~any(isnan(pp))
      plot(pp(1),pp(2),'om');
    end
  end
  
  title(sprintf('Camera %d',c));
  set(gca,'Position',get(gca,'OuterPosition'));   % Explode plot to fill space
end

% Convert a global coordinate into a camera pixel position
function pp=getpixelpos(cam,gpos)
Rwc=inv(cam.extcal.Rcw);
Twc=-Rwc*cam.extcal.Tcw;
scale=cam.hpixels/cam.distortion.Wcal;
gposCF=Rwc*gpos'+Twc;
if gposCF(3)>0
  pp=round(scale*distort(cam.distortion,(gposCF(1:2)/gposCF(3))')');
  if pp(1)<0||pp(1)>=cam.hpixels||pp(2)<0||pp(2)>=cam.vpixels
    pp=[nan,nan];
  end
else
  pp=[nan,nan];
end

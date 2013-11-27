% Check pixcalib structure against camera distortion measurements
function summary=camdistcheck(p)
d(1)=struct('fc',[ 1620.33241   1616.55291 ]/2,'cc',[ 1886.8300   1293.5384 ]/2,'kc',[ -0.063     0.004696  -0.002996   0.002624 ]);
d(2)=struct('fc',[ 1602.62137   1602.23155 ]/2,'cc',[ 2053.08166  1259.10321]/2,'kc',[ -0.06183   0.00166   -0.00124   -0.00023 ]);
d(3)=struct('fc',[ 1608.57522   1606.29564 ]/2,'cc',[ 1944.60839  1267.31895]/2,'kc',[ -0.05717   0.00375   -0.00526    0.00120 ]);
d(4)=struct('fc',[ 1628.54220   1625.30708 ]/2,'cc',[ 1853.74221  1221.23994]/2,'kc',[ -0.05573  -0.00919    0.00414   -0.00089 ]);
d(5)=struct('fc',[ 1614.21882   1613.03981 ]/2,'cc',[ 2048.10572  1260.01432]/2,'kc',[ -0.05630  -0.00641    0.00568   -0.00247 ]);
d(6)=struct('fc',[ 1627.21340   1625.98615 ]/2,'cc',[ 1982.12803  1228.71600]/2,'kc',[ -0.07807   0.02728   -0.01733    0.00312]);
ridmap=[1,2,6,5,3,4];   % data is p.camera(i) is for physical camera idmap(i)
rmax=10;
r=0:.01:rmax;
r(2,:)=0;
rd=[];
setfig('Distortion');clf;
col='rgbcmy';
for i=1:length(d)
  rdist=apply_fisheye_distortion(r,d(i).kc);
  rd(i,:)=rdist(1,:);
  plot(atand(r(1,:)),rd(i,:)*1620,col(i)); hold on;
end
keysizes=[3648,2752,norm([3648,2752])]/2;
for i=1:length(keysizes)
  plot(atand([0,rmax]),[keysizes(i),keysizes(i)],':');
end
xlabel('Angle (degrees)');
ylabel('Pixel position');

for cam=1:6
  distdata=d(idmap(cam));
  camheight=-0.18;   % Height of cameras relative to plane of LEDs
  pc=p.camera(cam).pixcalib;
  lnum=[p.led.id];
  % Setup camera position and orientation
  campos=[p.layout.cpos(cam,:),camheight];
  %campos(2)=campos(2)+.05;
  campos(1)=campos(1)-0.0;
  [camtheta,camphi]=cart2sph(p.layout.cdir(cam,1),p.layout.cdir(cam,2),0);
  camroll=0;
  margin=200;


  pos=[];
  for i=1:length(pc)
    pos(i,:)=pc(i).pos;
    mappos(i,:)=undistort(distdata,pos(i,:));
    ledpos(i,:)=[p.layout.lpos(i,:),0];
  end

  
  % Select "good" led entries
  goodradius=tand(70);   % maximum 70deg off center point of lens
  radius=sqrt(mappos(:,1).^2+mappos(:,2).^2);
  sel=radius<goodradius;
  fprintf('Using %d/%d LED points with radius less than %.2f (within %.0f degrees of center)\n', sum(sel), length(sel), goodradius,atand(goodradius));

  % Iterate
  for iter=1:1
    % Calculate [theta,phi] (observed) and [theta,phi,r] (expected) from camera to target LED
    fprintf('Iteration %d, Camera position=[%.2f,%.2f,%.2f], Camera angle=[%.2f,%.2f,%.2f]',  iter, campos, camtheta,camphi,camroll);
    rmappos=camerarotate(mappos,camroll);
    theta=atan(rmappos(:,1)');    % These are relative to the camera's frame of reference
    phi=atan(rmappos(:,2)');
    ltheta=[];lphi=[];calcphys=[];dcalcphys=[];
    for i=1:length(pc)
      vec=ledpos(i,:)-campos;
      [ltheta(i),lphi(i),lr(i)]=cart2sph(vec(1),vec(2),vec(3));
      % Convert to camera frame of reference
      ltheta(i)=camtheta-ltheta(i);
      lphi(i)=camphi-lphi(i);
      % Position on ideal rectilinear sensor
      calcphys(i,:)=[tan(ltheta(i)),tan(lphi(i))];
      % Predicted position on actual camera sensor
      dcalcphys(i,:)=distort(distdata,camerarotate(calcphys(i,:),-camroll));
    end


    
    % Update camera aim
    thetacorrection=nanmedian(ltheta(sel)-theta(sel));
    camtheta=camtheta-thetacorrection;

    phicorrection=nanmedian(lphi(sel)-phi(sel));
    camphi=camphi-phicorrection;
    
    e1=sum((camerarotate(calcphys(sel,:),-0)-rmappos(sel,:)).^2);
    rollcorrection=fminsearch(@(z) sum(sum((camerarotate(calcphys(sel,:),-z)-rmappos(sel,:)).^2)),0);
    %  transform=calcphys(sel,:)\rmappos(sel,:);
    %  rollcorrection=atan((transform(1,2)-transform(2,1))/(transform(1,1)+transform(2,2)));
    camroll=camroll+rollcorrection;

    % Update camera location
    for i=1:length(pc)
      [dx,dy,dz]=sph2cart(camtheta-theta(i),camphi-phi(i),lr(i));
      cpos(i,:)=ledpos(i,:)-[dx,dy,dz];
    end
    calcpos=nanmean(cpos(sel,:));
    deltapos=campos-calcpos;
    
    campos=calcpos;
    setfig('poserr');clf;
    cc=cpos; cc(~sel,:)=nan;
    plot(lnum,campos(1)-cc(:,1),'r');hold on;
    plot(lnum,campos(2)-cc(:,2),'g');
    plot(lnum,campos(3)-cc(:,3),'b');
    ylabel('Position error');


    fprintf(' Corr: dir=[%.3g,%.3g,%.3g], pos=[%.3g,%.3g,%.3g]\n',thetacorrection,phicorrection,rollcorrection,deltapos);
  end

  options=optimset('Display','iter','MaxFunEvals',5000,'MaxIter',5000);
  zinit=[camtheta,camphi,camroll,campos];
  zfinal=fminsearch(@(z) sqrt(nanmean(cerror(distdata,z(1:3),z(4:6),ledpos(sel,:),pos(sel,:)).^2)),zinit,options);
  fprintf('Preminimizer pos=[%.2f,%.2f,%.2f], dir=[%.2f,%.2f,%.2f]\n', zinit(4:6),zinit(1:3));
  fprintf('Minimizer pos=[%.2f,%.2f,%.2f], dir=[%.2f,%.2f,%.2f]\n', zfinal(4:6),zfinal(1:3));
  campos=zfinal(4:6);
  camtheta=zfinal(1);
  camphi=zfinal(2);
  camroll=zfinal(3);
  errsel=cerror(distdata,zfinal(1:3),zfinal(4:6),ledpos(sel,:),pos(sel,:));
  [err,ltheta,lphi,lr,calcphys,dcalcphys]=cerror(distdata,zfinal(1:3),zfinal(4:6),ledpos,pos);
  totalerr=sqrt(nanmean(errsel.^2));
  rmappos=camerarotate(mappos,camroll);
  theta=atan(rmappos(:,1)');    % These are relative to the camera's frame of reference
  phi=atan(rmappos(:,2)');

  % Calculate LED positions assuming camera accurate
  calcledpos=[];
  for i=1:length(pc)
    [dx,dy,dz]=sph2cart(camtheta-theta(i),camphi-phi(i),lr(i));
    calcledpos(i,:)=campos+[dx,dy,dz];
  end

  % Unwrap angles, convert to degrees
  theta=unwrap(theta)*180/pi;
  ltheta=unwrap(ltheta)*180/pi;
  if nanmean(theta)>90
    theta=theta-180;   % Flipped over
  end
  phi=unwrap(phi)*180/pi;
  lphi=unwrap(lphi)*180/pi;
  if nanmean(phi)>90
    phi=phi-180;   % Flipped over
  end

  thetaerror=nanstd(ltheta(sel)-theta(sel));
  phierror=nanstd(lphi(sel)-phi(sel));
  pixerror=sqrt(mean((dcalcphys(sel,1)-pos(sel,1)).^2+(dcalcphys(sel,2)-pos(sel,2)).^2));
  if abs(pixerror-totalerr)>.01
    fprintf('pixerror=%f, totalerr=%f\n',pixerror,totalerr);
    keyboard;
  end
  summary(cam)=struct('campos',campos,'camdir',[camtheta,camphi,camroll],'ledpos',calcledpos,'phierr',phierror,'thetaerr',thetaerror,'pixerr',pixerror,'minimizererr',totalerr,'pcerr',err);
  
  % Plots
  setfig('LEDs');clf;
  plot(ledpos(:,1),ledpos(:,2),'-');
  hold on;
  for i=1:length(pc)
    if sel(i)
      if norm(ledpos(i,:)-calcledpos(i,:))>.02
        col='r';
      else
        col='g';
      end
      plot([ledpos(i,1),calcledpos(i,1)],[ledpos(i,2),calcledpos(i,2)],['-',col]);
    end
  end
  plot(campos(1),campos(2),'xr');
  [dvec(1),dvec(2)]=pol2cart(camtheta,0.3);
  plot(campos(1)+[0,dvec(1)],campos(2)+[0,dvec(2)],'-r');
  axis equal
  title(sprintf('LED positions  (RMSE=%.3f m)',sqrt(mean((ledpos(sel,1)-calcledpos(sel,1)).^2+(ledpos(sel,2)-calcledpos(sel,2)).^2))));

  setfig('camdistcheck.1');clf;
  plot(pos(sel,1),pos(sel,2),'.g','MarkerSize',1);hold on;
  plot(dcalcphys(sel,1),dcalcphys(sel,2),'.b','MarkerSize',1);
  plot(distdata.cc(1),distdata.cc(2),'xr');
  legend('All LEDs','Expected','Optical Center','Location','SouthEast');
  title(sprintf('Raw pixel positions (RMS error=%.1f pixels)',pixerror));

  axis equal
  axis([0,p.camera(cam).hpixels-1,0,p.camera(cam).vpixels-1]);
  setfig('camdistcheck.2');clf;
  plot(rmappos(sel,1),rmappos(sel,2),'.g','MarkerSize',1);hold on;
  axis equal
  plot(calcphys(sel,1),calcphys(sel,2),'.b','MarkerSize',1);
  for i=1:size(rmappos,1)
    if sel(i)
      plot([rmappos(i,1),calcphys(i,1)],[rmappos(i,2),calcphys(i,2)],'-');
    end
  end
  for i=1:20:size(rmappos,1)
    if sel(i)
      text(rmappos(i,1)+0.2,rmappos(i,2),sprintf('%d',i));
    end
  end
  legend('All LEDs','Expected','Location','SouthEast');
  title('Undistorted normalized sensor positions');

  setfig('camdistcheck.3');clf;
  subplot(211);
  plot(lnum(sel),theta(sel),'.','MarkerSize',1);
  hold on;
  plot(lnum(sel),ltheta(sel),'g.','MarkerSize',1);
  legend('Observed','Computed');
  xlabel('LED');
  ylabel('Theta (degrees)');
  title(sprintf('Computed/Expected Theta (Std=%.2f degrees)',thetaerror));

  subplot(212);
  plot(lnum(sel),mod(theta(sel)-ltheta(sel)+180,360)-180,'r.','MarkerSize',1);
  xlabel('LED');
  ylabel('Theta error (degrees)');
  title(sprintf('Error in Theta (Std=%.2f degrees)',thetaerror));

  setfig('camdistcheck.4');clf;
  subplot(211);
  plot(lnum(sel),phi(sel),'.','MarkerSize',1);
  hold on;
  plot(lnum(sel),lphi(sel),'g.','MarkerSize',1);
  legend('Observed','Computed');
  xlabel('LED');
  ylabel('Phi (degrees)');
  title(sprintf('Computed/Expected Phi (Std=%.2f degrees)',phierror));

  subplot(212);
  plot(lnum(sel),mod(phi(sel)-lphi(sel)+180,360)-180,'r.','MarkerSize',1);
  xlabel('LED');
  ylabel('Phi error (degrees)');
  title(sprintf('Error in Phi (Std=%.2f degrees)',phierror));

end


function x=undistort(d,xd)
xd=(xd-d.cc)./d.fc;
x=comp_fisheye_distortion(xd',d.kc)';

function xd=distort(d,x)
x=apply_fisheye_distortion(x',d.kc)';
xd=x.*d.fc+d.cc;


% Rotate point in x,y plane by theta radians ccw
function rp=camerarotate(p,theta)
[th,r]=cart2pol(p(:,1),p(:,2));
[rp(:,1),rp(:,2)]=pol2cart(th+theta,r);

% Calculate pixel errors on sensor between calculated and observed sensor positions
function [err,ltheta,lphi,lr,calcphys,dcalcphys]=cerror(distdata,camdir,campos,ledpos,sensorpos)
for i=1:size(ledpos,1)
  vec=ledpos(i,:)-campos;
  [ltheta(i),lphi(i),lr(i)]=cart2sph(vec(1),vec(2),vec(3));
  % Convert to camera frame of reference
  ltheta(i)=camdir(1)-ltheta(i);
  lphi(i)=camdir(2)-lphi(i);
  % Position on ideal rectilinear sensor
  calcphys(i,:)=[tan(ltheta(i)),tan(lphi(i))];
  % Predicted position on actual camera sensor
  dcalcphys(i,:)=distort(distdata,camerarotate(calcphys(i,:),-camdir(3)));
  err(i)=norm(dcalcphys(i,:)-sensorpos(i,:));
end


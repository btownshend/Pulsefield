% Check RMS pixel error in camera model
function [d,dorig]=cameramodel(p,d)
% Evaluate camera model
  if nargin<2
    % Initialize estimate of parameters
    d=struct();
    d.lpos=p.layout.lpos;
    d.lpos(:,3)=30*2.54/100;   % Assume at 30" high
    d.cpos=p.layout.cpos;
    d.cpos(:,3)=d.lpos(1,3)-2*2.54/100;   % Assume 2" lower
    d.ra(:,3)=atan2(p.layout.cdir(:,1),p.layout.cdir(:,2));
    d.ra(:,2)=0;
    d.ra(:,1)=0;
    d.fx=[];d.fy=[];d.cx=[];d.cy=[];
    for i=1:length(p.camera)
      d.fx(i)=860; % (p.camera(i).hpixels/2) / atan(p.camera(i).fov/2);
      d.fy(i)=d.fx(i);
      d.cx(i)=p.camera(i).hpixels/2;
      d.cy(i)=p.camera(i).vpixels/2;
    end

    % Radial and tangential distortion
    % Initialize to values extracted from OpenCV chessboard calibration
    if 0
      d.distCoeff=[-0.32917 0.08178 0 0 -0.0080941 0 0 0];   
      %d.fx(2)=1016.23;
      %d.fy(2)=d.fx(2);
      %d.cx(2)=999.287;
      %d.cy(2)=642.427;
    elseif 0
      d.distCoeff=[.12999351 -.013499205 0 0 -0.00040558506 .512373786 -0.01454526 -0.003065358]; % 1+k(1)r^2+k(2)r^4+k(3)r^6/(1+k(4)r^2+k(5)r^4+k(6)r^7
      %d.fx(:)=917.347;
      %d.fy(:)=d.fx(2);
      %d.cx(2)=1009.19;
      %d.cy(2)=639.868;
    else
      d.distCoeff=[-0.02053879296615514, 0.00400819973773613, 0.00124440454693342, 0.004301244972504056, -0.002753600461101195, 0.3381550206938602, -0.02376399467159672, -0.005505159990399851];
      d.fx(2)=940.819;
      d.fy(2)=d.fx(2);
      d.cx(2)=954.578;
      d.cy(2)=631.127;
    end
    
    %testmodel(d);
    % testpack(d);
  end
  
  % Plot lens distortion function
  setfig('distortion'); clf;
  undistortplot(d.distCoeff,d.fx(2),p.camera(2).hpixels,p.camera(2).vpixels);
  
  % Convert observed pixel positions of LEDS into a matrix pos(nc,nl,2)
  for i=1:length(p.camera)
    for j=1:length(p.camera(i).pixcalib)
      pc=p.camera(i).pixcalib(j);
      pos(i,j,:)=pc.pos;
      dist=norm(d.cpos(i,:)-d.lpos(j,:));
      frac=pos(i,j,1)/p.camera(i).hpixels;
      posinner(i,j,:)=pos(i,j,:);
      % Ignore points near edge of field of view, or too close to camera, or in entryway
      if frac<0.05 || frac>0.95 || j<42 || j>699-42  || dist<1
        posinner(i,j,:)=nan;
      end
    end
  end
  
  dorig=d;
  x=pack(d);

  % Plot predicted, observed pixel positions
  printd(d,dorig);
  setfig('cameramodel before');
  clf;
  plotpixpos(d,pos,p);

  for i=1:7
    fprintf('\n**** Iteration %d\n', i);
    if i==1
      d=optim_cam(d,posinner,'fa');
    elseif i==2
      d=optim_global(d,pos);
      setfig('distortion'); hold on;
      undistortplot(d.distCoeff,d.fx(2),p.camera(2).hpixels,p.camera(2).vpixels,'r');
    elseif i==3
      %      d=optim_led(d,pos,'z');
      d=optim_cam(d,posinner,'zfca');
    elseif i==4
      % Should only adjust camera positions if we have parallax points (near LED strip and far one)
      % Should check for this and only optimize those ones
      d=optim_cam(d,pos,'xyz');
    elseif i==5
      d=optim_cam(d,posinner,'fca');
    elseif i==6
      d=optim_cam(d,pos,'xyza');
    elseif i==7
      d=optim_global(d,pos);
      setfig('distortion'); hold on;
      undistortplot(d.distCoeff,d.fx(2),p.camera(2).hpixels,p.camera(2).vpixels,'g');
    elseif i==4
      d=optim_led(d,posinner,'z');
      d=optim_cam(d,pos,'zfa');
    elseif i==5
      d=optim_cam(d,pos,'xyza');
    elseif i==6
      d=optim_all(d,pos);
    end
    
    printd(d,dorig);
    setfig(sprintf('cameramodel iter %d',i));
    clf;
    plotpixpos(d,pos,p);
    pause(0.1);

    if i==10
      % Remove outliers
      pix=pixestimate(d);
      err=sqrt(sum((pix-pos).^2,3));
      maxerr=50;
      for j=1:size(err,1)
        for k=1:size(err,2)
          if err(j,k)>maxerr
            fprintf('Removed outlier: camera %d, led %d, distance=%.1f pixels\n', j, k, err(j,k));
            pos(j,k,:)=nan;
          end
        end
      end
      fprintf('Removed %d outliers with error > %f\n',sum(err(:)>maxerr),maxerr);
    end
    plottopview(p,d,pos,i);
  end


  d.pix=pixestimate(d);
  d.pos=pos;
  end

  function plottopview(p,d,pos,iter)
    ti=sprintf('Layout Iteration %d',iter);
    setfig(ti); clf;
    plot(p.layout.cpos(:,1),p.layout.cpos(:,2),'xr');
    hold on;
    %  plot(p.layout.lpos(:,1),p.layout.lpos(:,2),'.r');
    plot(d.cpos(:,1),d.cpos(:,2),'xg');
    rotangle=d.ra(:,3);
    for i=1:size(d.cpos,1)
      plot(d.cpos(i,1)+[0,0.4*sin(rotangle(i))],d.cpos(i,2)+[0,0.4*cos(rotangle(i))],'g');
      [~,minp]=nanmin(pos(i,:,1));
      [~,maxp]=nanmax(pos(i,:,1));
      plot([d.cpos(i,1),d.lpos(minp,1),nan,d.lpos(maxp,1),d.cpos(i,1)],[d.cpos(i,2),d.lpos(minp,2),nan,d.lpos(maxp,2),d.cpos(i,2)],'m');
    end
    plot(d.lpos(:,1),d.lpos(:,2),'.g');
    for i=1:size(d.lpos,1)
      plot([d.lpos(i,1),p.layout.lpos(i,1)],[d.lpos(i,2),p.layout.lpos(i,2)]);
    end
    title(iter);
    xlabel('X');
    ylabel('Y');
    axis equal
  end

  function printd(d,dorig)
    nc=size(d.cpos,1);
    nl=size(d.lpos,1);
    fprintf(  'Radial distortion = 1+%gr^2+%gr^4+%gr^6\n',d.distCoeff([1,2,5]));
    fprintf('                  /(1+%gr^2+%gr^4+%gr^6)\n',d.distCoeff(6:8));
    fprintf('Tangential distortion = (%g,%g)\n',d.distCoeff(3:4));

    for i=1:nc
      fprintf('Camera %d: fx,fy=%.0f,%.0f cx,cy=%.0f,%.0f, ra=%.1f,%.1f,%.1f deg, pos=%.2f,%.2f,%.2f (offset=%.2f,%.2f,%.2f)\n',i, d.fx(i),d.fy(i),d.cx(i),d.cy(i),d.ra(i,:)*180/pi,d.cpos(i,:),d.cpos(i,:)-dorig.cpos(i,:));
    end
    for i=1:100:nl
      fprintf('LED %d: pos=%.2f,%.2f,%.2f (offset=%.2f,%.2f,%.2f)\n', i, d.lpos(i,:),d.lpos(i,:)-dorig.lpos(i,:));
    end
  end
  
% Optimize (optim flags = x:cposx,y:cposy,z:cposz,f:flen,c:center,a:angles
function d=optim_cam(d,pos,optim)
  fprintf('Optimizing camera-specific parameters (%s)\n',optim);
  nc=size(d.cpos,1);
  for i=1:nc
    dsel=packsel(d);
    if ismember('x',optim) dsel.cpos(i,1)=1; end
    if ismember('y',optim) dsel.cpos(i,2)=1; end
    if ismember('z',optim) dsel.cpos(i,3)=1; end
    if ismember('f',optim) dsel.fy(i)=1; dsel.fx(i)=1; end
    if ismember('c',optim) dsel.cx(i)=1; dsel.cy(i)=1; end     % These may be aliased with the absolute position of the camera
    if ismember('a',optim) dsel.ra(i,:)=1; end
    fprintf('Camera %d ...',i);
    di=d;
    d=optimize(d,dsel,pos,optimset('MaxFunEvals',4000,'MaxIter',10000),false,i);
    [olderr,oldseperr]=calcerror(di,pos,i);
    [newerr,newseperr]=calcerror(d,pos,i);
    fprintf('error went from %f to %f (sep=%f)\n', sqrt(olderr), sqrt(newerr),sqrt(newseperr));
  end
end

% Optimize LEDs (x:posx, y:posy, z:posz)
function d=optim_led(d,pos,optim)
  fprintf('Optimizing led-specific parameters (%s)\n',optim);
  nl=size(d.lpos,1);
  dselinit=packsel(d);
  joint=0;
  if joint
    dsel=dselinit;
    if ismember('x',optim) dsel.lpos(:,1)=1; end
    if ismember('y',optim) dsel.lpos(:,2)=1; end
    if ismember('z',optim) dsel.lpos(:,3)=1; end
    di=d;
    d=optimize(d,dsel,pos,optimset('MaxFunEvals',10000),false);
    for i=1:nl
      if mod(i-1,100)==0 || i==nl || i==1
        fprintf('LED %d error went from %f to %f\n', i, sqrt(calcerror(di,pos,[],i)), sqrt(calcerror(d,pos,[],i)));
      end
    end
  else
    for i=1:nl
      dsel=dselinit;
      if ismember('x',optim) dsel.lpos(i,1)=1; end
      if ismember('y',optim) dsel.lpos(i,2)=1; end
      if ismember('z',optim) dsel.lpos(i,3)=1; end
      di=d;
      d=optimize(d,dsel,pos,optimset('MaxFunEvals',1000),false,[],i);
      if mod(i-1,100)==0 || i==nl || i==1
        fprintf('LED %d error went from %f to %f\n', i, sqrt(calcerror(di,pos,[],i)), sqrt(calcerror(d,pos,[],i)));
      end
    end
  end
end

function d=optim_global(d,pos)
  fprintf('Optimizing global parameters\n');
  dsel=packsel(d);
  dsel.distCoeff=d.distCoeff~=0;
  dsel.fx(:)=1;
  dsel.fy(:)=1;
  dsel.ra(:,:)=1;
  d=optimize(d,dsel,pos,optimset('MaxFunEvals',100000,'MaxIter',100000,'Display','final'));
end

function d=optim_all(d,pos)
  fprintf('Optimizing all parameters\n');
  full=pack(d);
  full(3:end)=1;
  dsel=unpack(full);
  d=optimize(d,dsel,pos,optimset('MaxFunEvals',10000));
end

function df=optimize(d,dsel,pos,options,msg,cam,led);
  if nargin<4
    options=optimset('MaxFunEvals',1000);
  end
  if nargin<5
    msg=true;
  end
  if nargin<6
    cam=[];
  end
  if nargin<7
    led=[];
  end
  x=pack(d);
  xsel=logical(pack(dsel));
  xsel(1:2)=false;  % Sizes
  xfixed=x(~xsel);
  x0=x(xsel);
  %fprintf('Initial error=%f\n', sqrt(calcerror(unpack(x0,xsel,xfixed),pos,cam,led)));
  ierr=sqrt(calcerror(unpack(x0,xsel,xfixed),pos,cam,led));
  if isnan(ierr)
    %fprintf('Skipping optimize: error=NaN\n');
    df=d;
    return;
  end
  
  xf=fminsearch(@(x) calcerror(unpack(x,xsel,xfixed),pos,cam,led), x0,options);
  if msg
    [newerr,newseperr]=calcerror(unpack(xf,xsel,xfixed),pos,cam,led);
    fprintf('Total error went from %f to %f (seperr=%f)\n', ierr, sqrt(newerr),sqrt(newseperr));
  end
  xfull=[];
  xfull(xsel)=xf;
  xfull(~xsel)=xfixed;
  df=unpack(xfull);
end

% Pack paraemeter structure into an array
function x=pack(d)
  x=[size(d.cpos,1);size(d.lpos,1);d.fx(:);d.fy(:);d.cx(:);d.cy(:);d.distCoeff(:);d.cpos(:);d.ra(:);d.lpos(:)];
end

function dsel=packsel(d)
  empty=pack(d);
  empty(3:end)=0;
  dsel=unpack(empty);
end

function d=unpack(x,xsel,xfixed)
  if nargin==3
    z(xsel)=x;
    z(~xsel)=xfixed;
    x=z;
  end
  nc=x(1);
  nl=x(2);
  x=x(3:end);
  d=struct();
  d.fx=x(1:nc)';x=x(nc+1:end);
  d.fy=x(1:nc)';x=x(nc+1:end);
  d.fy=d.fx;   % Kludge force them to the same value (square pixels)
  d.cx=x(1:nc)';x=x(nc+1:end);
  d.cy=x(1:nc)';x=x(nc+1:end);
  d.distCoeff=x(1:8)';x=x(9:end);
  d.cpos=reshape(x(1:nc*3),nc,3);x=x(3*nc+1:end);
  d.ra=reshape(x(1:nc*3),nc,3);x=x(3*nc+1:end);
  d.lpos=reshape(x(1:nl*3),nl,3);
end

% Calculate MSE given d struct of parameters and pos(nc,nl,2)~pixel position of led on camera
function [mse,seperr]=calcerror(d,pos,cam,led)
  if nargin<3 || isempty(cam)
    cam=1:size(d.cpos,1);
  end
  if nargin<4 || isempty(led)
    led=1:size(d.lpos,1);
  end
  pix=pixestimate(d,cam,led);
  err=pos(cam,led,:)-pix;
  e2=err(:,:,1).^2+err(:,:,2).^2;
  mse=nanmean(e2(:));

  % Add in inter-led separation as an error term
  sep=diff(d.lpos);
  sepnorm=sqrt(sep(:,1).^2+sep(:,2).^2+sep(:,3).^2);
  sepnorm(numled(1)-1+[0,160,320,480,640])=nan;   % Between strips
  seperr=nanmean((sepnorm-nanmedian(sepnorm)).^2);
  % Approx 5 pixels = 1/32m, each led error results in ~4 camera errors
  seperr=seperr*(5/(1/32))^2*4;
  %fprintf('mse=%f, seperr=%f\n',mse,seperr);
  mse=mse+seperr;
end

% Plot comparision between observed and estimated pixel positions
function plotpixpos(d,pos,p)
  nc=size(d.cpos,1);
  nl=size(d.lpos,1);
  pix=pixestimate(d);
  clf;
  for i=1:nc
    row=floor((i-1)/3);
    col=i-row*3;
    subplot(6,3,row*9+col+[0,3]);
    for j=1:nl
      if isfinite(pos(i,j,1))
        plot(pos(i,j,1),pos(i,j,2),'.g'); hold on;
        plot(pix(i,j,1),pix(i,j,2),'.r');
        plot([pos(i,j,1),pix(i,j,1)],[pos(i,j,2),pix(i,j,2)],'m');
      end
    end
    for j=1:100:nl
      text(pos(i,j,1),pos(i,j,2)+100,sprintf('L%d',j));
      plot(pos(i,j,1)+[0,0],pos(i,j,2)+[0,80],'c');
    end
    plot([0,p.camera(i).hpixels-1,p.camera(i).hpixels-1,0,0],[0,0,p.camera(i).vpixels-1,p.camera(i).vpixels-1,0],':b');
    c=[0 p.camera(i).hpixels-1 0 p.camera(i).vpixels-1]+[-100 100 -100 100];
    axis ij
    axis equal
    axis(c);
    xlabel('X (pixels)');
    ylabel('Y (pixels)');
    err2=(pix(i,:,1)-pos(i,:,1)).^2 + (pix(i,:,2)-pos(i,:,2)).^2;
    rmse=sqrt(nanmean(err2));
    title(sprintf('Camera %d RMSE=%.1f\n', i, rmse));
    fprintf('Camera %d RMS error is %.1f pixels\n', i, rmse);
    % Error plot
    subplot(6,3,row*9+col+6);
    plot(pix(i,:,1)-pos(i,:,1),'r');
    hold on;
    plot(pix(i,:,2)-pos(i,:,2),'g');
    if i==1
      legend('X','Y');
    end
    xlabel('LED');
    ylabel('Error');
  end
end
      
function mapped=distort(d,i,tgts)
  x=tgts(:,1)./tgts(:,2);
  y=-tgts(:,3)./tgts(:,2);
  % Correct for radial distortion
  r2=(x.^2+y.^2);
  num=1+(d.distCoeff(1)+(d.distCoeff(2)+d.distCoeff(5)*r2).*r2).*r2;
  denom=1+(d.distCoeff(6)+(d.distCoeff(7)+d.distCoeff(8)*r2).*r2).*r2;
  rdistort=num./denom;
  % And tangential
  xd=x.*rdistort+2*d.distCoeff(3)*x.*y+d.distCoeff(4)*(r2+2*x.^2);
  yd=y.*rdistort+2*d.distCoeff(4)*x.*y+d.distCoeff(3)*(r2+2*y.^2);;
  % Move to optical center of sensor
  u=d.fx(i)*xd+d.cx(i);  % Why /2
  v=d.fy(i)*yd+d.cy(i);
  mapped=[u,v];
end
  
% Calculate pix(nc,nl,2) - estimated pixel position of led on camera based on model
function pix=pixestimate(d,cam,led)
  nc=size(d.cpos,1);
  nl=size(d.lpos,1);
  if nargin<2 || isempty(cam)
    cam=1:nc;
  end
  if nargin<3 || isempty(led)
    led=1:nl;
  end
  pix=zeros(length(cam),length(led),2);
  tgts=[];
  for ii=1:length(cam)
    i=cam(ii);
    rmat=[1 0 0;0 cos(d.ra(i,1)) sin(d.ra(i,1));0 -sin(d.ra(i,1)) cos(d.ra(i,1))]*[cos(d.ra(i,2)) 0 -sin(d.ra(i,2));0 1 0; sin(d.ra(i,2)) 0 cos(d.ra(i,2))]*[cos(d.ra(i,3)) sin(d.ra(i,3)) 0; -sin(d.ra(i,3)) cos(d.ra(i,3)) 0; 0 0 1];
    tgts(:,1)=d.lpos(led,1)-d.cpos(i,1);
    tgts(:,2)=d.lpos(led,2)-d.cpos(i,2);
    tgts(:,3)=d.lpos(led,3)-d.cpos(i,3);
    tgts=tgts*rmat;
    pix(ii,:,:)=distort(d,i,tgts);
  end
end

% Test distortion model implementation by checking some fixed points that can compared using openCV (with calibrate.cpp)
function testmodel(d)
  testpts=[0 0 0
           0 0 1
           0 0.01 1
           0.01 0 1
           0 1 0
           0 1 1 
           1 1 0
           1 1 .01
           1 1 1
           1 1.5 1
           1 2 1
           2 1 1
           1 1 2
           -1 1 1];
  testptsmodified=[testpts(:,1),testpts(:,3),-testpts(:,2)];
  dpos=distort(d,2,testptsmodified);
  for i=1:size(testpts,1)
    fprintf('[%f,%f,%f] maps to [%f,%f]\n', testpts(i,:), dpos(i,:));
  end
end

% Test pack routine
function testpack(d)
  x=pack(d);
  dd=unpack(x);
  x2=pack(dd);
  if any(x~=x2 & isfinite(x) & isfinite(x2))
    error('Pack failure\n');
  end
end

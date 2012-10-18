% Check RMS pixel error in camera model
function [d,dorig]=cameramodel(p)
% Evaluate camera model
% Initialize estimate of parameters
  d=struct();
  d.lpos=p.layout.lpos;
  d.lpos(:,3)=30*2.54/100;   % Assume at 30" high
  d.cpos=p.layout.cpos;
  d.cpos(:,3)=d.lpos(1,3)-4*2.54/100;   % Assume 4" lower
  d.ra(:,3)=atan2(p.layout.cdir(:,1),p.layout.cdir(:,2));
  d.ra(:,2)=0;
  d.ra(:,1)=0;
  d.fx=[];d.fy=[];d.cx=[];d.cy=[];
  for i=1:length(p.camera)
    d.fx(i)=(p.camera(i).hpixels/2) / atan(p.camera(i).fov/2);
    d.fy(i)=d.fx(i);
    d.cx(i)=p.camera(i).hpixels/2;
    d.cy(i)=p.camera(i).vpixels/2;
  end
  % Radial distortion
  d.radial=[0 0 0 0];   % 1+k(1)r^2+k(2)r^4+k(3)r^6...

  % Convert observed pixel positions of LEDS into a matrix pos(nc,nl,2)
  for i=1:length(p.camera)
    for j=1:length(p.camera(i).pixcalib)
      pc=p.camera(i).pixcalib(j);
      pos(i,j,:)=pc.pos;
    end
  end
  
  % Test pack
  if 1
    x=pack(d);
    dd=unpack(x);
    x2=pack(dd);
    if any(x~=x2 & isfinite(x) & isfinite(x2))
      error('Pack failure\n');
    end
  end
  
  dorig=d;
  x=pack(d);

  % Plot predicted, observed pixel positions
  printd(d,dorig);
  setfig('cameramodel before');
  clf;
  plotpixpos(d,pos,p);

  for i=1:10
    fprintf('Iteration %d\n', i);
    d=optim_cam(d,pos);
    d=optim_global(d,pos);
    if i>5
      d=optim_led(d,pos);
    end

    printd(d,dorig);
    setfig(sprintf('cameramodel iter %d',i));
    clf;
    plotpixpos(d,pos,p);
    pause(0.1);

    if i==5
      pix=pixestimate(d);
      err=sqrt(sum((pix-pos).^2,3));
      maxerr=100;
      for j=1:size(err,1)
        for k=1:size(err,2)
          if err(j,k)>maxerr
            pos(j,k,:)=nan;
          end
        end
      end
      fprintf('Removed %d outliers with error > %f\n',sum(err(:)>maxerr),maxerr);
    end
  end


  d.pix=pixestimate(d);
  d.pos=pos;
end

function printd(d,dorig)
  nc=size(d.cpos,1);
  nl=size(d.lpos,1);
  fprintf('Radial distortion = 1+%gr^2+%gr^4+%gr^6+%gr^8\n',d.radial);
  for i=1:nc
    fprintf('Camera %d: fx,fy=%.0f,%.0f cx,cy=%.0f,%.0f, pos=%.2f,%.2f,%.2f\n',i, d.fx(i),d.fy(i),d.cx(i),d.cy(i),d.cpos(i,:));
  end
  for i=1:100:nl
    fprintf('LED %d: pos=%.2f,%.2f,%.2f (offset=%.2f,%.2f,%.2f)\n', i, d.lpos(i,:),d.lpos(i,:)-dorig.lpos(i,:));
  end
end
    
function d=optim_cam(d,pos)
  fprintf('Optimizing camera-specific parameters\n');
  nc=size(d.cpos,1);
  for i=1:nc
    dsel=packsel(d);
    dsel.fy(i)=1;
    dsel.fx(i)=1;
    dsel.cx(i)=0; % These may be aliased with the absolute position of the camera
    dsel.cy(i)=0;
    dsel.cpos(i,:)=1;
    dsel.ra(i,:)=1;
    fprintf('Camera %d...',i);
    d=optimize(d,dsel,pos,optimset('MaxFunEvals',4000));
  end
end

function d=optim_led(d,pos)
  fprintf('Optimizing led-specific parameters\n');
  nl=size(d.lpos,1);
  dselinit=packsel(d);
  for i=1:nl
    dsel=dselinit;
    dsel.lpos(i,3)=1;   % Only the z-coordinate for now
    if mod(i-1,100)==0 || i==nl || i==1
      fprintf('LED%d ',i);
      msg=true;
    else
      msg=false;
    end
    d=optimize(d,dsel,pos,optimset('MaxFunEvals',1000),msg);
  end
end

function d=optim_global(d,pos)
  fprintf('Optimizing global parameters\n');
  dsel=packsel(d);
  dsel.radial(:)=1;
  d=optimize(d,dsel,pos);
end

function df=optimize(d,dsel,pos,options,msg);
  if nargin<4
    options=optimset('MaxFunEvals',1000);
  end
  if nargin<5
    msg=true;
  end
  x=pack(d);
  xsel=logical(pack(dsel));
  xsel(1:2)=false;  % Sizes
  xfixed=x(~xsel);
  x0=x(xsel);
  %fprintf('Initial error=%f\n', sqrt(calcerror(unpack(x0,xsel,xfixed),pos)));

  xf=fminsearch(@(x) calcerror(unpack(x,xsel,xfixed),pos), x0,options);
  if msg
    fprintf('Final error=%f\n', sqrt(calcerror(unpack(xf,xsel,xfixed),pos)));
  end
  xfull=[];
  xfull(xsel)=xf;
  xfull(~xsel)=xfixed;
  df=unpack(xfull);
end

% Pack paraemeter structure into an array
function x=pack(d)
  x=[size(d.cpos,1);size(d.lpos,1);d.fx(:);d.fy(:);d.cx(:);d.cy(:);d.radial(:);d.cpos(:);d.ra(:);d.lpos(:)];
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
  d.radial=x(1:4)';x=x(5:end);
  d.cpos=reshape(x(1:nc*3),nc,3);x=x(3*nc+1:end);
  d.ra=reshape(x(1:nc*3),nc,3);x=x(3*nc+1:end);
  d.lpos=reshape(x(1:nl*3),nl,3);
end

% Calculate MSE given d struct of parameters and pos(nc,nl,2)~pixel position of led on camera
function mse=calcerror(d,pos)
  pix=pixestimate(d);
  err=pos-pix;
  mse=nanmean(err(:).^2);
end

% Plot comparision between observed and estimated pixel positions
function plotpixpos(d,pos,p)
  nc=size(d.cpos,1);
  nl=size(d.lpos,1);
  pix=pixestimate(d);
  for i=1:nc
    subplot(2,ceil(nc/2),i);
    for j=1:nl
      if isfinite(pos(i,j,1))
        plot(pos(i,j,1),pos(i,j,2),'.g'); hold on;
        plot(pix(i,j,1),pix(i,j,2),'.r');
        plot([pos(i,j,1),pix(i,j,1)],[pos(i,j,2),pix(i,j,2)],'m');
      end
    end
    c=axis;
    plot([0,p.camera(i).hpixels-1,p.camera(i).hpixels-1,0,0],[0,0,p.camera(i).vpixels-1,p.camera(i).vpixels-1,0],':b');
    axis(c);
    xlabel('X (pixels)');
    ylabel('Y (pixels)');
    rmse=sqrt(nanmean((pix(i,:,1)-pos(i,:,1)).^2 + (pix(i,:,2)-pos(i,:,2)).^2));
    title(sprintf('Camera %d RMSE=%.1f\n', i, rmse));
    fprintf('Camera %d RMS error is %.1f pixels\n', i, rmse);
  end
end
      
% Calculate pix(nc,nl,2) - estimated pixel position of led on camera based on model
function pix=pixestimate(d)
  nc=size(d.cpos,1);
  nl=size(d.lpos,1);
  pix=zeros(nc,nl,2);
  for i=1:nc
    rmat=[1 0 0;0 cos(d.ra(i,1)) sin(d.ra(i,1));0 -sin(d.ra(i,1)) cos(d.ra(i,1))]*[cos(d.ra(i,2)) 0 -sin(d.ra(i,2));0 1 0; sin(d.ra(i,2)) 0 cos(d.ra(i,2))]*[cos(d.ra(i,3)) sin(d.ra(i,3)) 0; -sin(d.ra(i,3)) cos(d.ra(i,3)) 0; 0 0 1];
    tgts=(d.lpos-repmat(d.cpos(i,:),nl,1))*rmat;
    x=d.fx(i)/2*tgts(:,1)./tgts(:,2);
    y=-d.fy(i)/2*tgts(:,3)./tgts(:,2);
    % Correct for radial distortion
    r2=(x.^2+y.^2)/1e6;   % Scale to be approx. 1 at edges (just to keep constants in reasonable scale)
    rdistort=1+d.radial(1)*r2+d.radial(2)*r2.^2+d.radial(3)*r2.^3+d.radial(4)*r2.^4;
    x=x./rdistort;
    y=y./rdistort;
    % Move to optical center of sensor
    x=x+d.cx(i);
    y=y+d.cy(i);

    pix(i,:,1)=x;
    pix(i,:,2)=y;
  end
end

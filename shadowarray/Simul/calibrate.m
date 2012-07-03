% Calibrate positions of camera's LED's
% Use spos(c,l) - sensor position (linear) of led l in camera c, null if not visible
% Normalize to distance between LED's
% Unknowns are:
%  cp(ncamera,2) - camera positions
%  cd(ncamera,2) - camera direction
%  lp(nled, 2) - led positions
% Data is:
%  spos(ncamera,nled) - position of LED on sensor in pixels
%  p - camera, setup parameters
% Approach:
%  initial estimate of all variables
%  build linear system relating changes in positions of each variable needed to correct observed sensor angles
%  solve system
%  repeat until stable
% Pass in true cpos, cdir, lpos to allow tracking of errors during simulation
function calib=calibrate(p,layout,spos,doplot)
if nargin<4
  doplot=false;
end
ncamera=size(spos,1);
nled=size(spos,2);

% Convert spos to sa (angles)
for c=1:size(spos,1)
  sa(c,:)=spos2angle(spos(c,:),p.camera(c));
end

% Distort lens a bit
sa=sa*1.00;

% initialize camera positions randomly offset from actual ones with 5cm stdev
cp=layout.cpos+rand(size(layout.cpos))*.0;
% But fix first 2 cameras to eliminate scale/rotation of frame of reference variation
cp(1:2,:)=layout.cpos(1:2,:);

cd=layout.cdir+rand(size(layout.cdir))*.01;
% normalize direction vectors
for i=1:ncamera
  cd(i,:)=cd(i,:)/norm(cd(i,:));
end

% initialize LED positions randomly offset from actual ones with 1cm stdev
lp=layout.lpos+randn(size(layout.lpos))*.0;

errtarget=1e-10;

% Plot vectors from actual to estimated
if doplot
  setfig('calibrate.initial');
  clf;
  hold on;
  for c=1:ncamera
    plot([cp(c,1),layout.cpos(c,1)],[cp(c,2),layout.cpos(c,2)],'r');
    plot(cp(c,1)+50*[0,cd(c,1)],cp(c,2)+50*[0,cd(c,2)],'b');
    plot(layout.cpos(c,1)+50*[0,layout.cdir(c,1)],layout.cpos(c,2)+50*[0,layout.cdir(c,2)],'m');
  end
  for l=1:10:nled
    plot([lp(l,1),layout.lpos(l,1)],[lp(l,2),layout.lpos(l,2)],'g');
  end
  title(sprintf('Actual to estimated - initial\n'));
  pause(0);
end

% Find delta for each component to move to observed spos
% All of the dlp,dcp,dcd give the avg amount to move to fix spos by 1 pixel
calib=layout;
calib.cpos=cp; calib.cdir=cd; calib.lpos=lp;
[espos,esa]=calcspos(p,calib,1,1);
sel=isfinite(spos(:)+espos(:));
serr=norm(spos(sel)-espos(sel));
lastserr=serr;

% Check error
fprintf('Iter 0: RMS error in layout.lpos=%f, layout.cpos=%f, layout.cdir=%f, spos=%f\n', norm(layout.lpos-lp), norm(layout.cpos-cp), norm(layout.cdir-cd),serr);

rmse=[];


for iter=1:500
  % Calculate direction vectors from camera to LED
  for c=1:ncamera
    ang=atan2(cd(c,2),cd(c,1));
    % Add led position angles  
    dir(c,:,1)=cos(sa(c,:)+ang);
    dir(c,:,2)=sin(sa(c,:)+ang);
  end

  dsa=esa-sa;

  % Setup a system of equations Ax=b
  % Where x=[dcp : dga : dlp] where x,y are interleaved
  maxpts=ncamera*nled;
  npts=0;
  A=zeros(maxpts,3*ncamera+2*nled);b=zeros(maxpts,1);
  for c=1:ncamera
    sel=find(isfinite(sa(c,:)));
    for ls=1:length(sel)
      l=sel(ls);
      dist=norm(cp(c,:)-lp(l,:));
      % We know that cp(c,:)-lp(l,:) should change by dist in direction orthogonal to dir(c,l,:)
      npts=npts+1;
      % Contribution due to error in camera position
      A(npts,2*c-1)=-dir(c,l,2);
      A(npts,2*c)=dir(c,l,1);
      % Contribution due to error in gaze angle
      A(npts,2*ncamera+c)=dist;
      % Contribution due to error in led position
      A(npts,3*ncamera+2*l-1)=dir(c,l,2);
      A(npts,3*ncamera+2*l)=-dir(c,l,1);
      b(npts)=dsa(c,l)*dist;
    end
  end
  % Fix location of 2 cameras to lock rotation/scaling down
  A(npts+1,1)=1;
  b(npts+1)=layout.cpos(1,1)-cp(1,1);
  A(npts+2,2)=1;
  b(npts+2)=layout.cpos(1,2)-cp(1,2);
  A(npts+3,3)=1;
  b(npts+3)=layout.cpos(2,1)-cp(2,1);
  A(npts+4,4)=1;
  b(npts+4)=layout.cpos(2,2)-cp(2,2);
  npts=npts+4;
  
  A=A(1:npts,:); b=b(1:npts,1);
  fprintf('Solving %d equations in %d unknowns\n',size(A));
  x=A\b;   % Jointly solve for cp,lp

  dcp=[x(1:2:ncamera*2),x(2:2:ncamera*2)];
  dga=x(ncamera*2+1:ncamera*3);
  dlp=[x(ncamera*3+1:2:end),x(ncamera*3+2:2:end)];
  dcd= [cos(dga).*cd(:,1)-sin(dga).*cd(:,2), sin(dga).*cd(:,1)+cos(dga).*cd(:,2)] - cd;

  k=[0 1 0];
  % Update all variables by k*delta
  while max(k)>1e-10
    % Find delta for each component to move to observed spos
    % All of the dlp,dcp,dcd give the avg amount to move to fix spos by 1 pixel
    calib.cpos=cp+k(1)*dcp; calib.cdir=cd+k(2)*dcd; calib.lpos=lp+k(3)*dlp;
    [espos,esa]=calcspos(p,calib,1,1);
    sel=isfinite(spos(:)+espos(:));
    serr=norm(spos(sel)-espos(sel));
    if serr<lastserr
      break;
    end
    k=k/2;
    fprintf('Error increasing to %f -- changed k to [%f, %f, %f]\n',serr, k);
  end

  % Check error
  rmse(iter,1:3)=[norm(layout.lpos-lp)/sqrt(length(lp(:))), norm(layout.cpos-cp)/sqrt(length(cp(:))), norm(layout.cdir-cd)/sqrt(length(cd(:)))];
  rmse(iter,4)= serr;

  fprintf('Iter %d: RMS error in layout.lpos=%f, cpos=%f, layout.cdir=%f, spos=%f\n', iter, rmse(iter,:));
  if serr>lastserr
    fprintf('Divergence!\n');
    break;
  end
    
  lastserr=serr;
  cp=cp+k(1)*dcp;
  cd=cd+k(2)*dcd;
  lp=lp+k(3)*dlp;
  if serr<errtarget
    fprintf('Converged to error %f\n', serr);
    break;
  end
end

if doplot
  % Plot all points and delta
  setfig('calibrate.deltas');
  clf;
  hold on;

  for c=1:ncamera
    plot(cp(c,1)+[0,dcp(c,1)],cp(c,2)+[0,dcp(c,2)],'g');
    plot(cp(c,1),cp(c,2),'go');
    plot(layout.cpos(c,1),layout.cpos(c,2),'gx');
  end
  for l=1:20:nled
    plot(lp(l,1)+[0,dlp(l,1)],lp(l,2)+[0,dlp(l,2)],'r');
    plot(lp(l,1),lp(l,2),'ro');
    plot(layout.lpos(l,1),layout.lpos(l,2),'rx');
  end
  title('calibrate.deltas');
  
  % Plot vectors from actual to estimated
  setfig('calibrate.estimates');
  clf;
  hold on;
  for c=1:ncamera
    dirlen=0.1;	% Length of direction vectors to draw (in meters)
    plot([cp(c,1),layout.cpos(c,1)],[cp(c,2),layout.cpos(c,2)],'r');
    plot(cp(c,1)+dirlen*[0,cd(c,1)],cp(c,2)+dirlen*[0,cd(c,2)],'b');
    plot(layout.cpos(c,1)+dirlen*[0,layout.cdir(c,1)],layout.cpos(c,2)+dirlen*[0,layout.cdir(c,2)],'m');
  end
  for l=1:10:nled
    plot([lp(l,1),layout.lpos(l,1)],[lp(l,2),layout.lpos(l,2)],'g');
  end
  title(sprintf('Actual to estimated, iteration %d\n', iter));

  setfig('calibrate.rmse');
  semilogy(rmse,'-');
  xlabel('Iteration');
  ylabel('RMSE');
  legend('lpos','cpos','cdir','spos');
  title('RMSE');
  
  setfig('calibrate.sensorerr');
  clf;
  plot(espos'-spos');
  xlabel('LED');
  ylabel('Sensor position error (pixel)');
  title(sprintf('Iteration %d',iter));
  
  setfig('calibrate.lederr');
  clf;
  subplot(211);
  plot((lp(:,1)-layout.lpos(:,1))*100,'.','MarkerSize',1);
  title('X Position error for LEDs');
  xlabel('LED');
  ylabel('Error (cm)');
  subplot(212);
  plot((lp(:,2)-layout.lpos(:,2))*100,'.','MarkerSize',1);
  title('Y Position error for LEDs');
  xlabel('LED');
  ylabel('Error (cm)');

  setfig('calibrate.camerr');
  clf;
  subplot(211);
  plot((cp(:,1)-layout.cpos(:,1))*100,'o');
  title('X Position error for cameras');
  xlabel('Camera');
  ylabel('Error (cm)');
  subplot(212);
  plot((cp(:,2)-layout.cpos(:,2))*100,'o');
  title('Y Position error for cameras');
  xlabel('Camera');
  ylabel('Error (cm)');
end
calib=layout;
calib.cpos=cp; calib.cdir=cd; calib.lpos=lp;

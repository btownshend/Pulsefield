% Calibrate positions of camera's LED's
% Use spos(c,l) - sensor position (linear) of led l in camera c, null if not visible
% Normalize to distance between LED's
% Unknowns are:
%  cp(ncameras,2) - camera positions
%  cd(ncameras,2) - camera direction
%  lp(nled, 2) - led positions
% Data is:
%  spos(ncameras,led) - position of LED on sensor in pixels
%  sp2ang(sensorpos) - mapping from sensor position in pixels to angle relative to cd()
% Approach:
%  initial estimate of all variables
%  fix cp,cd, find optimal position of each lp
%  fix lp, find optimal positions of each cp, cd 
%  repeat until stable
function [cp,cd,lp,espos]=calibrate3(spos,p,cpos,cdir,lpos)
ncamera=size(spos,1);
nled=size(spos,2);
%nled=50;

% Convert spos to sa (angles)
sa=(spos/p.cam.hpixels-0.5)*p.cam.fov;
sa=sa(:,1:nled);

% initialize camera positions randomly offset from actual ones with 5cm stdev
cp=cpos+rand(size(cpos))*.01*p.scale;
cd=cdir+rand(size(cdir))*.02;
% normalize direction vectors
for i=1:ncamera
  cd(i,:)=cd(i,:)/norm(cd(i,:));
end

% initialize LED positions randomly offset from actual ones with 1cm stdev
lp=lpos+randn(size(lpos))*.01*p.scale;

doplot=1;
errtarget=0.1;

% Find delta for each component to move to observed spos
% All of the dlp,dcp,dcd give the avg amount to move to fix spos by 1 pixel
[espos,esa]=calcspos(cp,cd,lp,p,1,1);
sel=isfinite(spos(:));
if sum(~isfinite(espos(sel)))
  error('NAN values in espos unexpected\n');
end
serr=norm(spos(sel)-espos(sel));
lastserr=serr;

% Check error
fprintf('Iter 0: RMS error in lpos=%f, cpos=%f, cdir=%f, spos=%f\n', norm(lpos-lp), norm(cpos-cp), norm(cdir-cd),serr);

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
  dlp=0*lp;nlp=dlp;
  dcp=0*cp;ncp=dcp;
  dcd=0*cd;ncd=dcd;
  if doplot>1
    figure(12);
    clf;
    axis equal
    hold on;
  end

  for c=1:ncamera
    sel=find(isfinite(sa(c,:)));
    for ls=1:length(sel)
      l=sel(ls);
      dist=norm(cp(c,:)-lp(l,:));
      wt=abs(dir(c,l,:));
      dlp(l,1)=dlp(l,1)+dsa(c,l)*dir(c,l,2)*dist *wt(2);   % Weighted adjustment
      nlp(l,1)=nlp(l,1)+wt(2); % Weight alone
      dlp(l,2)=dlp(l,2)-dsa(c,l)*dir(c,l,1)*dist * wt(1);
      nlp(l,2)=nlp(l,2)+wt(1);

      dcp(c,1)=dcp(c,1)-dsa(c,l)*dir(c,l,2)*dist * wt(2);
      ncp(c,1)=ncp(c,1)+wt(2);
      dcp(c,2)=dcp(c,2)+dsa(c,l)*dir(c,l,1)*dist * wt(1);
      ncp(c,2)=ncp(c,2)+wt(1);

      dcd(c,1)=dcd(c,1)-dsa(c,l)*cd(c,2);
      ncd(c,1)=ncd(c,1)+1;
      dcd(c,2)=dcd(c,2)+dsa(c,l)*cd(c,1);
      ncd(c,2)=ncd(c,2)+1;

      if c==1 && ls==0
        ta=acos(dot(cpos(c,:)-lp(l,:),cp(c,:)-lp(l,:))/norm(cpos(c,:)-lp(l,:))/norm(cp(c,:)-lp(l,:)));
        fprintf('led=%d\ncpos=(%f,%f)\ncurr=(%f,%f)\nlpos=(%f,%f)\ndir =(%f,%f)\ndist=%f\ndsa =%f\nta  =%f\ndcp =(%f,%f)\nncp =(%f,%f)\n', l, cpos(c,:), cp(c,:), lpos(l,:), dir(c,l,:), dist, dsa(c,l), ta, dcp(c,:), ncp(c,:));
        if doplot>1
          plot(cp(c,1),cp(c,2),'go'); 
          plot(cpos(c,1),cpos(c,2),'gx');
          %        plot(lp(l,1),lp(l,2),'ro');
          plot(cp(c,1)+[0,dcp(c,1)]/ncp(c,1),cp(c,2)+[0,dcp(c,2)]/ncp(c,2),'g');
          plot(cp(c,1)+[0,dir(c,l,1)]/10,cp(c,2)+[0,dir(c,l,2)]/10,'r');
        end
      end
    end
  end
  dlp=dlp./nlp;
  dcp=dcp./ncp;
  dcd=dcd./ncd;
  
  fprintf('max(abs(dcd)) = %f\n', max(abs(dcd(:))));
  if iter<4
    fprintf('Updating gaze only\n');
    k=[0 1 0];
%  elseif max(abs(dlp(:))) > max(abs(dcp(:)))
%    k=[0 0 1];
  else
    k=[1 0 1];
  end
  % Update all variables by k*delta
  while max(k)>1e-6
    % Find delta for each component to move to observed spos
    % All of the dlp,dcp,dcd give the avg amount to move to fix spos by 1 pixel
    [espos,esa]=calcspos(cp+k(1)*dcp,cd+k(2)*dcd,lp+k(3)*dlp,p,1,1);
    sel=isfinite(spos(:));
    fprintf('sum(sel)=%d\n',sum(sel));
    serr=norm(spos(sel)-espos(sel));
    if serr<lastserr
      break;
    end
    k=k/2;
    fprintf('Error increasing to %f -- changed k to [%f, %f, %f]\n',serr, k);
  end

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

  

  % Check error
  rmse(iter,1:3)=[norm(lpos-lp)/sqrt(length(lp(:))), norm(cpos-cp)/sqrt(length(cp(:))), norm(cdir-cd)/sqrt(length(cd(:)))];
  rmse(iter,4)= serr;

  fprintf('Iter %d: RMS error in lpos=%f, cpos=%f, cdir=%f, spos=%f\n', iter, rmse(iter,:));
end

% Rescale and center
for j=1:4
  refcenter=mean(lpos);
  refsz=mean(abs(lpos(:,1)-refcenter(1))+abs(lpos(:,2)-refcenter(2)));
  refangle=mean(unwrap(atan2(lpos(:,2)-refcenter(2),lpos(:,1)-refcenter(1))));
  center=mean(lp);
  sz=mean(abs(lp(:,1)-center(1))+abs(lp(:,2)-center(2)));
  angle=mean(unwrap(atan2(lp(:,2)-center(2),lp(:,1)-center(1))));
  adjangle=-(refangle-angle);
  fprintf('Scaling by 1+%.2g, offset by (%.4f,%.4f), rotate by %.4f degrees\n', refsz/sz-1, center-refcenter, adjangle*180/pi);
  lp(:,1)=(lp(:,1)-center(1))*cos( adjangle)+(lp(:,2)-center(2))*sin(adjangle);
  lp(:,2)=(lp(:,1)-center(1))*sin(-adjangle)+(lp(:,2)-center(2))*cos(adjangle);
  cp(:,1)=(cp(:,1)-center(1))*cos( adjangle)+(cp(:,2)-center(2))*sin(adjangle);
  cp(:,2)=(cp(:,1)-center(1))*sin(-adjangle)+(cp(:,2)-center(2))*cos(adjangle);
  cd(:,1)=cd(:,1)*cos( adjangle)+cd(:,2)*sin(adjangle);
  cd(:,2)=cd(:,1)*sin(-adjangle)+cd(:,2)*cos(adjangle);
  for k=1:2
    lp(:,k)=lp(:,k)*refsz/sz + refcenter(k);
    cp(:,k)=cp(:,k)*refsz/sz + refcenter(k);
  end
end
rmse(end+1,1:3)=[norm(lpos-lp)/sqrt(length(lp(:))), norm(cpos-cp)/sqrt(length(cp(:))), norm(cdir-cd)/sqrt(length(cd(:)))];
rmse(end,4)= serr;

if doplot
  % Plot all points and delta
  figure(8);
  clf;
  hold on;

  for c=1:ncamera
    plot(cp(c,1)+[0,dcp(c,1)],cp(c,2)+[0,dcp(c,2)],'g');
    plot(cp(c,1),cp(c,2),'go');
    plot(cpos(c,1),cpos(c,2),'gx');
  end
  for l=1:20:nled
    plot(lp(l,1)+[0,dlp(l,1)],lp(l,2)+[0,dlp(l,2)],'r');
    plot(lp(l,1),lp(l,2),'ro');
    plot(lpos(l,1),lpos(l,2),'rx');
  end

  % Plot vectors from actual to estimated
  figure(9);
  clf;
  hold on;
  for c=1:ncamera
    plot([cp(c,1),cpos(c,1)],[cp(c,2),cpos(c,2)],'r');
    plot(cp(c,1)+50*[0,cd(c,1)],cp(c,2)+50*[0,cd(c,2)],'b');
    plot(cpos(c,1)+50*[0,cdir(c,1)],cpos(c,2)+50*[0,cdir(c,2)],'m');
  end
  for l=1:10:nled
    plot([lp(l,1),lpos(l,1)],[lp(l,2),lpos(l,2)],'g');
  end
  title(sprintf('Actual to estimated, iteration %d\n', iter));

  figure(10);
  semilogy(rmse,'-');
  xlabel('Iteration');
  ylabel('RMSE');
  legend('lpos','cpos','cdir','spos');

  figure(11);
  clf;
  plot(espos'-spos');
  xlabel('LED');
  ylabel('Sensor position error (pixel)');
  title(sprintf('Iteration %d',iter));
  
  figure(12);
  clf;
  subplot(211);
  plot((lp(:,1)-lpos(:,1))/p.scale*100,'.','MarkerSize',1);
  title('X Position error for LEDs');
  xlabel('LED');
  ylabel('Error (cm)');
  subplot(212);
  plot((lp(:,2)-lpos(:,2))/p.scale*100,'.','MarkerSize',1);
  title('Y Position error for LEDs');
  xlabel('LED');
  ylabel('Error (cm)');

  figure(13);
  clf;
  subplot(211);
  plot((cp(:,1)-cpos(:,1))/p.scale*100,'o');
  title('X Position error for cameras');
  xlabel('Camera');
  ylabel('Error (cm)');
  subplot(212);
  plot((cp(:,2)-cpos(:,2))/p.scale*100,'o');
  title('Y Position error for cameras');
  xlabel('Camera');
  ylabel('Error (cm)');
end

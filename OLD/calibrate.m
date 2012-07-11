% Calibrate positions of camera's LED's
% Use spos(c,l) - sensor position (linear) of led l in camera c, null if not visible
% Normalize to distance between LED's
% Unknowns are:
%  cp(ncamera,2) - camera positions
%  cd(ncamera,2) - camera direction
%  lp(nled, 2) - led positions
% Data is:
%  spos(ncamera,led) - position of LED on sensor in pixels
%  sp2ang(sensorpos) - mapping from sensor position in pixels to angle relative to cd()
% Approach:
%  initial estimate of all variables
%  fix cp,cd, find optimal position of each lp
%  fix lp, find optimal positions of each cp, cd 
%  repeat until stable
function [cp,cd,lp]=calibrate(spos,p,cpos,cdir,lpos)
ncamera=size(spos,1);
nled=size(spos,2);
%nled=50;

% Convert spos to sa (angles)
sa=(spos/p.cam.hpixels-0.5)*p.cam.fov;
sa=sa(:,1:nled);

% initialize camera positions randomly offset from actual ones with 5cm stdev
cp=cpos+rand(size(cpos))*.05*p.scale;
% initialize direction vectors randomly skewed
cd=cdir+rand(size(cdir))*.00;
for i=1:ncamera
  cd(i,:)=cd(i,:)/norm(cd(i,:));
end
% initialize LED positions randomly offset from actual ones with 1cm stdev
lp=lpos+randn(size(lpos))*.00*p.scale;

% Check error
fprintf('Iter 0: RMS error in lpos=%f, cpos=%f, cdir=%f\n', norm(lpos-lp), norm(cpos-cp), norm(cdir-cd));
rmse=[];
doplot=48;

for iter=1:50
  % Estimate lp
  % Calculate direction vectors to LEDs
  dir=[];


  % Calculate direction vectors from camera to LED
  for c=1:ncamera
    ang=atan2(cd(c,2),cd(c,1));
    % Add led position angles  
    dir(c,:,1)=cos(sa(c,:)+ang);
    dir(c,:,2)=sin(sa(c,:)+ang);
%    plot(cp(c,1),cp(c,2),'or');
%    plot(cp(c,1)+[0,cd(c,1)],cp(c,2)+[0,cd(c,2)],'r');
  end

  if mod(iter,3)==0
    % Estimate LED positions holding cameras fixed
    newlp=[];
    if iter>=doplot
      figure(8);
      clf;
      hold on;
      title(sprintf('Iteration %d LED position adjustment\n', iter));
    end
    
    for l=1:nled
      % Formulate as A*x=b
      A=[];b=[];
      sel=find(isfinite(dir(:,l,1)));
      for cs=1:length(sel)
        c=sel(cs);
        cs2=mod(cs,length(sel))+1;
        c2=sel(cs2);
        for k=1:2
          b(end+1,1)=cp(c,k)-cp(c2,k);
          A(end+1,cs)=-dir(c,l,k);
          A(end,cs2)=dir(c2,l,k);
        end
      end
      x=A\b;
      newlp(l,1)=mean(cp(sel,1)+x.*dir(sel,l,1));
      newlp(l,2)=mean(cp(sel,2)+x.*dir(sel,l,2));
      if iter>=doplot
        if mod(l,20)==0
          for cs=1:length(sel)
            c=sel(cs);
            plot(cp(c,1)+x(cs)*dir(c,l,1),cp(c,2)+x(cs)*dir(c,l,2),'b.');
          end
          plot(newlp(l,1),newlp(l,2),'go');
          plot([lp(l,1),newlp(l,1)],[lp(l,2),newlp(l,2)],'g');
        end
      end
    end
    lp=(lp+newlp)/2;
  elseif mod(iter,3)==2
    % Calculate positions of cameras holding LEDs fixed
    newcp=[];
    if iter>=doplot
      figure(7);
      clf;
      hold on;
      title(sprintf('Iteration %d camera position adjustment\n', iter));
    end

    for c=1:ncamera
      % Formulate as A*x=b
      sel=find(isfinite(dir(c,:,1)));
      A=zeros(2*length(sel),length(sel));
      b=zeros(2*length(sel),1);
      for ls=1:length(sel)
        l=sel(ls);
        ls2=mod(ls,length(sel))+1;
        l2=sel(ls2);
        for k=1:2
          b(end+1,1)=lp(l,k)-lp(l2,k);
          A(end+1,ls)=-dir(c,l,k);
          A(end,ls2)=dir(c,l2,k);
        end
      end
      x=A\b;
      newcp(c,1)=mean(lp(sel,1)+x.*dir(c,sel,1)');
      newcp(c,2)=mean(lp(sel,2)+x.*dir(c,sel,2)');
      if iter>=doplot
        % Plot
        for ls=1:1:length(sel)
          l=sel(ls);
          plot(lp(l,1)+x(ls)*dir(c,l,1),lp(l,2)+x(ls)*dir(c,l,2));
        end
        plot([cp(c,1),newcp(c,1)],[cp(c,2),newcp(c,2)],'g-');
        plot(newcp(c,1),newcp(c,2),'go');
      end  
    end
    cp=(cp+newcp)/2;
  else
    % Estimate camera direction vectors
    for c=1:ncamera
      sel=find(isfinite(sa(c,:)));
      angerr=[];
      ang=atan2(cd(c,2),cd(c,1));
      for ls=1:length(sel)
        l=sel(ls);
        c2l=lp(l,:)-cp(c,:);
        ang2lp=atan2(c2l(2),c2l(1));
        angerr(ls)=ang-(ang2lp-sa(c,l));
      end
      angerr(angerr>pi)=angerr(angerr>pi)-2*pi;
      angerr(angerr<-pi)=angerr(angerr<-pi)+2*pi;
      newang=ang-mean(angerr);
      fprintf('Mean gaze angle of camera %d was %.2f degrees, changing by %.2f\n', c, ang*180/pi, (newang-ang)*180/pi);
      cd(c,:)=[cos(newang) sin(newang)];
      %    fprintf('Setting gaze of camera %d to (%f,%f)\n', c, cd(c,:));
    end
  end

  % Rescale and center
  for j=1:0
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
  
  % Plot vectors from actual to estimated
  if iter>=doplot
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
  end
  
  % Check error
  espos=calcspos(cp,cd,lp,p,1);
  rmse(iter,1:3)=[norm(lpos-lp)/sqrt(length(lp(:))), norm(cpos-cp)/sqrt(length(cp(:))), norm(cdir-cd)/sqrt(length(cd(:)))];
  sel=isfinite(spos(:))&isfinite(espos(:));
  rmse(iter,4)=norm(spos(sel)-espos(sel))/sqrt(sum(sel));
  fprintf('Iter %d: RMS error in lpos=%f, cpos=%f, cdir=%f, spos=%f\n', iter, rmse(iter,:));
  if iter>=doplot
    figure(11);
    clf;
    plot(espos'-spos');
    xlabel('LED');
    ylabel('Sensor position error (pixel)');
    title(sprintf('Iteration %d',iter));
    pause(1);
  end
end
figure(10);
semilogy(rmse);
xlabel('Iteration');
ylabel('RMSE');
legend('lpos','cpos','cdir','spos');


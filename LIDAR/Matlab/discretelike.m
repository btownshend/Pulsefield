% Discretize likelihood calculation to match scan to track
function discretelike(vis,track)
% Assume legdiam is log-normal (i.e. log(legdiam) ~ N(LOGDIAMMU,LOGDIAMSIGMA)
LOGDIAMMU=log(track.legdiam);
LOGDIAMSIGMA=log(1+DIAMSTD/track.legdiam);
d=exp([-10,log(0.01):0.02:log(1),10]);

xy=range2xy(vis.angle,vis.range);
xyfar=range2xy(vis.angle,vis.range+track.legdiam);
best=track.legs;
for i=1:2
  ldist(i,:)=sqrt((xy(:,1)-best(i,1)).^2+(xy(:,2)-best(i,2)).^2);
end
dist=min(ldist,[],1);
fsel=find(dist(1,:)<=MAXMOVEMENT & vis.bgprob<0.05);
% Find important rays that are not obstructed
sidelobes=20;
clearrange=[max(1,min(fsel)-sidelobes),min(length(vis.angle),max(fsel)+sidelobes)];
clearsel=clearrange(1):clearrange(2);
fprintf('Clear path for scans %d:%d\n',clearrange);
step=0.02;
for rep=1:2
  fprintf('Leg positions (%.2f,%.2f) and (%.2f,%.2f)\n',best(1,:),best(2,:));
  if (rep==1)
    % Split based on closest
    fs{1}=fsel(ldist(1,fsel)<=ldist(2,fsel));
    fs{2}=fsel(ldist(1,fsel)>ldist(2,fsel));
    for f=1:length(fsel)
      fprintf('%3d: d1=%.3f, d2=%.3f\n', fsel(f), ldist(:,fsel(f)));
    end
  else
    % Split based on likelihood from prior round
    fs={[],[]};
    ptlike=nan(length(fsel),2);
    for f=1:length(fsel)
      for i=1:2
        dpt=sqrt((best(i,1)-xy(fsel(f),1)).^2+(best(i,2)-xy(fsel(f),2)).^2);
        % This is a fudge since the DIAMETER is log-normal and the position itself is normal
        ptlike(f,i)=normlike([track.legdiam/2,DIAMSTD+measvar(i)],dpt);
      end
      p=exp(-ptlike(f,:));p=p/sum(p);
      fprintf('%3d: l1=%.1f, l2=%.1f, p1=%.3f, p2=%.3f\n', fsel(f), ptlike(f,:),p);
      if ptlike(f,1)<ptlike(f,2)
        fs{1}(end+1)=fsel(f);
      else
        fs{2}(end+1)=fsel(f);
      end
    end
  end
  minval=min(min(track.legs)-2*sqrt(track.posvar)-track.legdiam,min(xy(fsel,:),[],1)-2*step);
  maxval=max(max(track.legs)+2*sqrt(track.posvar)+track.legdiam,max(xyfar(fsel,:),[],1)+2*step);
  minval=floor(minval/step)*step;
  maxval=ceil(maxval/step)*step;
  nx=round((maxval(1)-minval(1))/step+1);
  xvals=minval(1)+(0:nx-1)*step;
  ny=round((maxval(2)-minval(2))/step+1);
  yvals=minval(2)+(0:ny-1)*step;
  fprintf('Searching over a %.0f x %.0f grid with %d,%d points/leg diam=%.2f +/- *%.2f\n', nx,ny,length(fs{1}),length(fs{2}),track.legdiam, exp(LOGDIAMSIGMA));

  % for ix=1:nx
  %   x=xvals(ix);
  %   for iy=1:ny
  %     y=yvals(iy);
  %     for i=1:2
  %       apriori(i,ix,iy)=normlike([0,sqrt(track.posvar(i))],norm(track.legs(i,:)-[x,y]));
  %       dpt=min(sqrt((xy(fs{i},1)-x).^2+(xy(fs{i},2)-y).^2));
  %       glike(i,ix,iy)=normlike([LOGDIAMMU,LOGDIAMSIGMA],log(dpt*2));
  %       clearlike=[];
  %       others=[clearsel,fs{3-i}];
  %       dclr=min(segment2pt([0,0],xy(others,:),[x,y]));
  %       %        for k=1:length(others)
  %       %          dclr(k)=segment2pt([0,0],xy(others(k),:),[x,y]);
  %       %end
  %       if dclr<farfromclear
  %            %clearlike=interp1(clrdist.d,clrdist.l,dclr);
  %         clearlike=-log(normcdf(log(dclr),LOGDIAMMU,LOGDIAMSIGMA));
  %         if isnan(clearlike)
  %           keyboard;
  %         end
  %         glike(i,ix,iy)=glike(i,ix,iy)+clearlike;
  %       end
  %       [th,r]=xy2range([x,y]);
  %       if any(squeeze(r<vis.range(fs{i})) & abs(vis.angle(fs{i})-th)'<res)
  %         glike(i,ix,iy)=1e99;
  %       end
  %     end
  %   end
  % end
  adist=nan(nx,ny);
  dclr=nan(nx,ny);
  glike=nan(2,nx,ny);
  apriori=nan(2,nx,ny);
  clearlike=nan(2,nx,ny);

  for i=1:2
    for ix=1:nx
      x=xvals(ix);
      for iy=1:ny
        y=yvals(iy);
        adist(ix,iy)=norm(track.legs(i,:)-[x,y]);
        dclr(ix,iy)=min(segment2pt([0,0],xy(clearsel,:),[x,y]));
        dpt=sqrt((xy(fs{i},1)-x).^2+(xy(fs{i},2)-y).^2);
        glike(i,ix,iy)=normlike([LOGDIAMMU,LOGDIAMSIGMA],log(dpt*2));
      end
    end
    apriori(i,:,:)=-log(normpdf(adist,0,sqrt(track.posvar(i))));
    clearlike(i,:,:)=-log(normcdf(log(dclr),LOGDIAMMU,LOGDIAMSIGMA));
  end

  glike=glike+clearlike;
  full=glike+apriori;

  for m=1:2
    bestlike=nan(1,2);
    for i=1:2
      [bestlike(i),bestind]=min(reshape(full(i,:,:),1,[]));
      [bestix,bestiy]=ind2sub([size(full,2),size(full,3)],bestind);
      best(i,:)=[minval(1)+step*(bestix-1),minval(2)+step*(bestiy-1)];
    end
    fprintf('Leg positions (%.2f,%.2f) with like=%f and (%.2f,%.2f) with like=%f, sep=%.2f\n',best(1,:), bestlike(1), best(2,:), bestlike(2),norm(best(1,:)-best(2,:)));

    % Calculate measurement error
    measvar=zeros(1,2);
    tlike=zeros(1,2);
    for ix=1:nx
      x=xvals(ix);
      for iy=1:ny
        y=yvals(iy);
        for i=1:2
          dpt2=(best(i,1)-x).^2+(best(i,2)-y).^2;
          p=exp(-full(i,ix,iy));
          measvar(i)=measvar(i)+dpt2*p;
          tlike(i)=tlike(i)+p;
        end
      end
    end
    measvar=measvar./tlike;
    fprintf('Measurement std %.2f and %.2f\n',sqrt(measvar));

    % Calculate separation likelihood using other leg at calculated position, variance
    sigma=sqrt(SEPSIGMA^2+measvar);
    fprintf('Computing separation likelikehood using sep=%.2f +/- [%.2f,%.2f]\n', SEPMU,sigma);
    seplike=nan(2,nx,ny);
    d=nan(nx,ny);
    for i=1:2
      for ix=1:nx
        x=xvals(ix);
        for iy=1:ny
          y=yvals(iy);
          d(ix,iy)=sqrt((best(i,1)-x).^2+(best(i,2)-y).^2);
        end
      end
      seplike(3-i,:,:)=-log(normpdf(d,SEPMU,sigma(i)));
    end
    % Repeat above calculations after updating full to include seplike
    full=glike+apriori+seplike;
  end
  
  setfig(sprintf('discretelike%d',rep));clf;
  [mx,my]=meshgrid(minval(2):step:maxval(2),minval(1):step:maxval(1));
  sym={'<','>'};
  for i=1:8
    lnum=floor((i-1)/4)+1;
    if mod(i-1,4)==0
      like=apriori(lnum,:,:);
      ti='Apriori';
    elseif mod(i-1,4)==1
      like=glike(lnum,:,:);
      ti='Measurement';
    elseif mod(i-1,4)==2
      like=seplike(lnum,:,:);
      ti='Sep';
    else
      like=full(lnum,:,:);
      ti='Composite';
    end
    subplot(2,4,i);
    pcolor(my,mx,squeeze(like));
    shading('interp');
    hold on;
    plot(xy(fs{lnum},1),xy(fs{lnum},2),['w',sym{lnum}]);
    plot(best(lnum,1),best(lnum,2),'wo');
    plot(track.legs(lnum,1),track.legs(lnum,2),'wx');
    title(sprintf('%s %d',ti,lnum));
    if mod(i-1,4)==1 || mod(i-1,4)==3
      caxis(min(like(:))+[0,30]);
    end
    axis equal
  end
  suptitle(sprintf('Frame %d',vis.frame));
end


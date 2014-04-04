% Discretize likelihood calculation to match scan to track
function track=discretelike(vis,track,fs)
params=getparams();
doplot=1;

% Assume legdiam is log-normal (i.e. log(legdiam) ~ N(LOGDIAMMU,LOGDIAMSIGMA)
LOGDIAMMU=log(track.legdiam);
LOGDIAMSIGMA=log(1+params.legdiamstd/track.legdiam);

% Maximum likelihood of a scan line being part of this target to use it
MAXPTLIKE=20;

xy=range2xy(vis.angle,vis.range);
best=track.legs;
measvar=track.posvar;
ldist=nan(2,size(xy,1));
for i=1:2
  ldist(i,:)=sqrt((xy(:,1)-best(i,1)).^2+(xy(:,2)-best(i,2)).^2);
end
dist=min(ldist,[],1);
fsel=find(dist(1,:)<=params.maxmovement & vis.bgprob<0.05);

step=0.02;

fprintf('Leg positions (%.2f,%.2f) and (%.2f,%.2f)\n',best(1,:),best(2,:));
% Bound search by prior position + 2*sigma(position) + legdiam/2
minval=min(track.legs)-2*sqrt(track.posvar)-track.legdiam/2;
maxval=max(track.legs)+2*sqrt(track.posvar)+track.legdiam/2;
fboth=[fs{1};fs{2}];
if ~isempty(fboth)
  % Make sure any potential measured point is also in the search
  minval=min(minval,min(xy(fboth,:),[],1));
  maxval=max(maxval,max(xy(fboth,:),[],1));
  xyfar=range2xy(vis.angle(fboth),vis.range(fboth)+track.legdiam);
  minval=min(minval,min(xyfar,[],1));
  maxval=max(maxval,max(xyfar,[],1));
end

minval=floor(minval/step)*step;
maxval=ceil(maxval/step)*step;

% Find the rays that will hit this box
[theta(1),~]=xy2range([maxval(1),minval(2)]);
[theta(2),~]=xy2range(minval);
[theta(3),~]=xy2range(maxval);
[theta(4),~]=xy2range([minval(1),maxval(2)]);
clearsel=find(vis.angle>=min(theta) & vis.angle<=max(theta));
fprintf('Clear path for scans %d:%d\n',min(clearsel),max(clearsel));


nx=round((maxval(1)-minval(1))/step+1);
xvals=minval(1)+(0:nx-1)*step;
ny=round((maxval(2)-minval(2))/step+1);
yvals=minval(2)+(0:ny-1)*step;
fprintf('Searching over a %.0f x %.0f grid with %d,%d points/leg diam=%.2f +/- *%.2f\n', nx,ny,length(fs{1}),length(fs{2}),track.legdiam, exp(LOGDIAMSIGMA));

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
  apriori(i,:,:)=-log(normpdf(adist,0,sqrt(track.posvar(i)+(params.sensorsigma^2))));
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
  sigma=sqrt(params.legsepstd^2+measvar);
  fprintf('Computing separation likelikehood using sep=%.2f +/- [%.2f,%.2f]\n', params.meanlegsep,sigma);
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
    seplike(3-i,:,:)=-log(normpdf(d,params.meanlegsep,sigma(i)));
  end
  % Repeat above calculations after updating full to include seplike
  full=glike+apriori+seplike;
end

if doplot
  setfig(sprintf('discretelike ID %d',track.id));clf;
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
  suptitle(sprintf('ID %d, Frame %d',track.id,vis.frame));
end 

track=track.clone();
track.prevlegs=track.legs;
track.legs=best;
track.position=mean(track.legs,1);
track.posvar=measvar;
% Check blockage as a function of number of people using monte carlo simulation
ncamera=7;	% Number of cameras
nsegs=8;	% Number of segments in pulsefield
tgtdiam=0.4;    % Target diameter
mingap=0.05;
tgtarea=pi*(tgtdiam/2)^2;
pctfunc=.9;	% Frac of Camera->LED lines that are functional
maxpeople=20;

nedges=ncamera*2;
ndata=2000;   % Data points per case
layout=layoutpolygon(nsegs,ncamera,0);

% Inset active area by tgtdiam/2
active=layout.active;
for i=1:size(active,1)
  dir=-active(i,:); dir=dir/norm(dir);
  active(i,:)=active(i,:)+dir*tgtdiam/2;
end

hnd=[];
lbls=[];
fracok=zeros(nedges+1,maxpeople);
fracok(:,1)=1;

setfig('blockmc'); clf; hold on;
cols='rgbcmykrgbcmykrgbcmykrgbcmykrgbcmyk';
for npeople=2:maxpeople;
  ntrials=ceil(ndata/npeople);
  nrand=ntrials*npeople*4;
  rnums=zeros(nrand,2);
  for i=1:2
    rnums(:,i)=random('unif',min(active(:,i)),max(active(:,i)),[nrand,1]);
  end
  rptr=1;
  sel=inpolygon(rnums(:,1),rnums(:,2),active(:,1),active(:,2));
  rnums=rnums(sel,:);
  nvis=zeros(1,nedges+1);
  for i=1:ntrials
    pos=zeros(npeople,2);
    for j=1:npeople
      bad=1;
      while bad
        pos(j,:)=rnums(rptr,:);rptr=rptr+1;
        bad=0;
        for k=1:j-1
          if norm(pos(k,:)-pos(j,:))<tgtdiam+mingap
            bad=1;
            break;
          end
        end
      end
    end
    if i==1
      plotlayout(layout);
      plot(active(:,1),active(:,2),'m');
      for k=1:size(pos,1)
        rectangle('Position',[pos(k,:)-tgtdiam/2,tgtdiam,tgtdiam],'Curvature',[1,1]);
      end
      setfig('blockmc');
    end
    for p=1:npeople
      nblock=0;
      for ic=1:ncamera
        ab1=[pos(p,:);layout.cpos(ic,:)]\[1+tgtdiam/2 1]';   % Equation of line through point
        ab2=[pos(p,:);layout.cpos(ic,:)]\[1-tgtdiam/2 1]';   % Equation of line through point
        blkd=[0 0];
        for p2=1:npeople
          if p2==p
            continue;
          end
          d=abs([(pos(p2,:)*ab1-1)/norm(ab1),(pos(p2,:)*ab2-1)/norm(ab2)]);   % Distance to edge lines 
                                                      % fprintf('D(%d,%d->%d)=%f %f\n', ic,p,p2,d);
          blkd=blkd|(d<tgtdiam/2);
        end
        nblock=nblock+sum(blkd);
      end
      % fprintf('Person %d blocked %d times\n',p,nblock);
      
      vindex=nedges-nblock+1;
      nvis(vindex)=nvis(vindex)+1;
    end
  end

  nvis=nvis/ntrials/npeople;
  hnd(end+1)=plot(nedges:-1:0,cumsum(nvis(end:-1:1)),cols(npeople-1));
  lbls{end+1}=sprintf('%d people',npeople);
  for i=1:length(nvis)
    fracok(i,npeople)=sum(nvis(i:end));
  end
end
xlabel('Min Edges visible');
ylabel('Fraction');
legend(hnd,lbls);

setfig('blockmc-frac');clf;hold on;
hnd=[];
lbls={};
for i=2:size(fracok,1)
  hnd(end+1)=plot(1:size(fracok,2),fracok(i,:),cols(i));
  lbls{end+1}=sprintf('>= %d edges\n', i-1);
end
legend(hnd,lbls);
axis([1,maxpeople,0,1]);
xlabel('Num People');
ylabel('Fraction of samples OK');
title(sprintf('Fraction of samples (ncamera=%d)',ncamera));



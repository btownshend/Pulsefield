% Compare the results of one multi run to another
function multicompare(varargin)
defaults=struct('nfiles',2,...
                'load',true,...
                'gtnum',1,...
                'gttrack',1 ...
                );
args=processargs(defaults,varargin);

m={};
for i=1:args.nfiles
  name=sprintf('multi%d',i);
  eval(sprintf('global %s',name));
  if (args.load)
    fprintf('Loading %s...', name);
    l=load(sprintf('../Recordings/%s-2000.mat',name));
    eval(sprintf('%s=l;',name));
    fprintf('done\n');
  end
  m{end+1}=eval(sprintf('%s.csnap',name));
end

% Build position maps p{mi}(i,j,k)  - multi mi, frame i, track j, x:k=1/y:k=2
p={};
for mi=1:length(m)
  frame=arrayfun(@(z) z.vis.frame, m{mi});
  p{mi}=nan(size(m{1},1),1,2);  % Reserve space for enough frame entries as multi1 has
  for i=1:length(frame)
    nid=length(m{mi}(i).tracker.tracks);
    for j=1:nid
      id=m{mi}(i).tracker.tracks(j).id;
      % NOTE: frame numbers in overlay files correspond to acquisition frames, the playback frames are never skipped
      % So, instead of using frame(i) as the first index below, use 'i' to keep things aligned
      p{mi}(i,id,:)=m{mi}(i).tracker.tracks(j).position;
    end
  end
  p{mi}(p{mi}(:)==0)=nan;
end

if size(p{1},2)>1
  fprintf('Warning: multi1 has more than 1 track, assuming ID 1 is the ground truth track\n');
end
groundtruth=squeeze(p{args.gtnum}(:,args.gttrack,:));
fprintf('Using multi%d ID %d as ground truth\n',args.gtnum,args.gttrack);

for i=1:args.gtnum-1
  p{i}(:)=nan;  % Blank out prior multis
end

% Find the track in each file corresponding to track 1 in multi1
for i=1:length(p)
  e=[];
  for j=1:size(p{i},2)
    diff=squeeze(groundtruth-squeeze(p{i}(:,j,:)));
    delta=diff(:,1).^2+diff(:,2).^2;
    e(j)=nansum(delta);
    fprintf('multi%d: track %d-GT err^2==%.1f mm\n', i,j,sqrt(e(j)/sum(isfinite(delta)))*1000);
  end
  [~,matched]=min(e);
  track(i,:,:)=squeeze(p{i}(:,matched,:));
end

setfig('multicompare'); clf;
mrkrsize=5;

subplot(3,1,1);
plot(track(:,:,1)','-','MarkerSize',mrkrsize);
xlabel('frame');
ylabel('x-position');
title('X position');

subplot(3,1,2);
plot(track(:,:,2)','-','MarkerSize',mrkrsize);
xlabel('frame');
ylabel('y-position');
title('Y Position');

subplot(3,1,3);
err=[];
for i=1:length(m)
  diff=squeeze(track(i,:,:))-groundtruth;
  err(i,:)=sqrt(diff(:,1).^2+diff(:,2).^2);
end
plot(err','-','MarkerSize',mrkrsize);
xlabel('frame');
ylabel('error');
tol=100/1000;
leg={};
totalbad=0;
for i=1:length(m)
  nbad=nansum(err(i,:)>tol);
  nframes=sum(isfinite(err(i,:)));
  fracbad=nbad/nframes;
  fprintf('multi%d: RMS error=%.1f mm, %.1f%% (%d) of  frames (%d) with >%.0fmm error\n', i, sqrt(nanmean(err(i,:).^2))*1000,100*fracbad,nbad,nframes,tol*1000);
  leg{i}=sprintf('multi%d - %.1f%% >%.0f mm',i,fracbad*100,tol*1000);
  totalbad=totalbad+nbad;
end
suptitle(datestr(now));
legend(leg);
title(sprintf('GT=multi%d.%d Error %.1f mm - Total bad frames=%d',args.gtnum, args.gttrack, sqrt(nanmean(err(:).^2))*1000, totalbad));

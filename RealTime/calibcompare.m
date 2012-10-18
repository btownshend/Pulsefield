% Compare multiple calibrations
function c=calibcompare(dirname,paths)
if nargin<1
  dirname='Calibration';
end
if nargin<2
  paths={'*.mat'};
end
if ~iscell(paths)
  paths={paths};
end
d=[];
for i=1:length(paths)
  d2=dir([dirname,'/',paths{i}]);
  if isempty(d2)
    fprintf('Warning: no files match %s/%s\n', dirname,paths{i});
  end
  d=[d;d2];
end
c={};
pctimes=[];
leg={};
for i=1:length(d)
  filename=[dirname,'/',d(i).name];
  fprintf('Loading %s..', filename);
  p=load(filename);
  if ~ismember(p.camera(1).pixcalibtime,pctimes)
    c{end+1}=p;
    pctimes(end+1)=p.camera(1).pixcalibtime;
    leg{end+1}=datestr(pctimes(end));
    fprintf('%s\n',leg{end});
  else
    fprintf('Skipping since it has same pixcalibtime as a prior one\n');
  end
end
for xy=1:2
setfig(sprintf('calibcompare %d',xy));
clf;
ncam=length(c{1}.camera);
for cam=1:ncam
  pos=nan(length(c),length(c{1}.camera(1).pixcalib));
  for i=1:length(c)
    pos(i,:)=arrayfun(@(z) z.pos(xy),c{i}.camera(cam).pixcalib);
  end
  subplot(ceil(ncam/2),2,cam);
  plot((pos-repmat(nanmedian(pos,1),length(c),1))','.');
  xlabel('LED');
  if xy==1
    ylabel('X-Pos delta');
  else
    ylabel('Y-Pos delta');
  end
  title(sprintf('Camera %d',cam));
  if cam==1
    legend(leg,'Location','Best')
  end
end
end

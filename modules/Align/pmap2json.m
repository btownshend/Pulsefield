% Convert a pmap from this aligner (scan + analyze) into JSON format for use by Calibrator

jsonin='/Users/bst/Dropbox/Pulsefield/src/modules/Calibrator/settings_proj.json';
jsonout='settings_proj_align.json';

fd=fopen(jsonin,'r');
if fd<0
  error('Unable to open %s',jsonin);
end
origjson=char(fread(fd))';
fclose(fd);
orig=jsondecode(origjson);
mappings=orig.calibration.mappings;
% Check current mappings
for i=1:length(mappings)
  m=mappings(i);
  m.unit1=str2double(m.unit1);
  m.unit2=str2double(m.unit2);
  if m.unit1>=length(pmap) || m.unit2>=length(pmap)
    continue;
  end
  fprintf('Mapping %d, units %d,%d\n',i,m.unit1,m.unit2);
  for j=1:length(m.pairs)
    p=m.pairs(j);
    p.locked=str2double(p.locked);
    p.pt1.x=str2double(p.pt1.x);
    p.pt1.y=str2double(p.pt1.y);
    p.pt2.x=str2double(p.pt2.x);
    p.pt2.y=str2double(p.pt2.y);
    if p.locked==1
      fprintf(' Pairs %d - (%.2f,%.2f) -> (%.2f,%.2f)\n',j, p.pt1.x,p.pt1.y,p.pt2.x,p.pt2.y);
      % Compare using pmap
      [cx,cy]=proj2cam(pmap{m.unit1+1},p.pt1.x/1920,p.pt1.y/1080);
      [px,py]=cam2proj(pmap{m.unit2+1},cx,cy);
      px=px*1920;py=py*1080;
      err=norm([px-p.pt2.x,py-p.pt2.y]);
      fprintf('   PMAP (%.2f,%.2f) -> (%.2f,%.2f)  err=%.1f pixels\n',p.pt1.x,p.pt1.y,px,py,err);
      [cx,cy]=proj2cam(pmap{m.unit2+1},p.pt2.x/1920,p.pt2.y/1080);
      [px,py]=cam2proj(pmap{m.unit1+1},cx,cy);
      px=px*1920;py=py*1080;
      err=norm([px-p.pt1.x,py-p.pt1.y]);
      fprintf('   PMAP (%.2f,%.2f) -> (%.2f,%.2f) err=%.1f pixels\n',px,py,p.pt2.x,p.pt2.y,err);
    end
  end
  % Choose some random points in unit1 and find their mapping to unit2
  nrand=1000;
  border=0.1;
  randpts1=rand(nrand,2)*(1-2*border)+border;
  [cx,cy]=proj2cam(pmap{m.unit1+1},randpts1(:,1),randpts1(:,2));
  randpts2=nan(size(randpts1));
  [randpts2(:,1),randpts2(:,2)]=cam2proj(pmap{m.unit2+1},cx,cy);
  sel=randpts2(:,1)>border & randpts1(:,1)<1-border & randpts2(:,2)>border & randpts2(:,2)<1-border;
  fprintf('Have %d/%d possible mappings\n', sum(sel), nrand);
  randpts1=randpts1(sel,:);
  randpts2=randpts2(sel,:);
  cx=cx(sel);
  cy=cy(sel);
  d1=pdist(randpts1);
  d2=pdist(randpts2);
  d=squareform((d1+d2)/2);
  npick=min(10,sum(sel));
  pick=[1];
  for j=2:npick
    d(:,pick)=nan;
    score=min(d(pick,:),[],1);
    [~,pick(end+1)]=max(score);
  end
  ti=sprintf('P%d vs P%d',m.unit1+1,m.unit2+1);
  setfig(ti);clf;
  plotcentroids(pmap{m.unit1+1},1);
  alpha(0.5);
  hold on;
  plotcentroids(pmap{m.unit2+1},1);
  alpha(0.5);
  plot(cx(pick),cy(pick),'ok');
  title(ti);
  % Set json to be new pairs
  pts1=randpts1(pick,:);pts2=randpts2(pick,:);
  pts1(:,1)=pts1(:,1)*1920;
  pts1(:,2)=pts1(:,2)*1080;
  pts2(:,1)=pts2(:,1)*1920;
  pts2(:,2)=pts2(:,2)*1080;
  newpairs=[];
  for j=1:npick
    newpairs=[newpairs,struct('pt1',struct('x',pts1(j,1),'y',pts1(j,2)),'pt2',struct('x',pts2(j,1),'y',pts2(j,2)),'locked',1)];
  end
  m.pairs=newpairs;
  mappings(i)=m;
end
keyboard
orig.calibration.mappings=mappings;
final=jsonencode(orig);
final=replace(final,',',[',',char(10)]);
fd=fopen(jsonout,'w');
fwrite(fd,final);
fclose(fd);

% Load an osc file 
function x=loadosc(filename)
fd=fopen(filename,'r');
if fd==-1
  error('Unable to open file: %s\n',filename);
end
x=[];
uidmap=[];
while 1
  line=fgetl(fd);
  if line==-1
    break;
  end
  fprintf('line=%s\n',line);
  blanks=find(line==' ');
  if length(blanks)<1
    continue
  end
  path=line(1:blanks(1)-1);
  if strcmp(path,'/pf/update')
    p=textscan(line,'%s %d %f %d %f %f %f %f %f %f %d %d %d');
    data=struct('frame',p{2},'time',p{3},'uid',p{4},'x',p{5},'y',p{6},'vx',p{7},'vy',p{8},'major',p{9},'minor',p{10},'groupid',p{11},'groupsize',p{12},'channel',p{13});
    uidpos=find(data.uid==uidmap,1);
    if isempty(uidpos)
      uidmap=[uidmap,data.uid];
      uidpos=length(uidmap);
    end
    x.uid(end,uidpos).update=data;
  elseif strcmp(path,'/pf/body')
    p=textscan(line,'%s %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d');
    data=struct('frame',p{2},'uid',p{3},'x',p{4},'y',p{5},'ex',p{6},'ey',p{7},'spd',p{8},'espd',p{9},'heading',p{10},'eheading',p{11},'facing',p{12},'efacing',p{13},'legdiam',p{14},'sigmadiam',p{15},'sep',p{16},'sigmasep',p{17},'leftness',p{18},'visibility',p{19});
    uidpos=find(data.uid==uidmap,1);
    if isempty(uidpos)
      uidmap=[uidmap,data.uid];
      uidpos=length(uidmap);
    end
    x.uid(end,uidpos).body=data;
  elseif strcmp(path,'/pf/frame')
    p=textscan(line,'%s %d');
    x=[x,struct('frame',p{2},'uid',[])];
  end
end

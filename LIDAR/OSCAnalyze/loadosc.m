% Load an osc file 
function x=loadosc(filename)
fd=fopen(filename,'r');
if fd==-1
  error('Unable to open file: %s\n',filename);
end
slash=find(filename=='/',1,'last');
if ~isempty(slash)
  filename=filename(slash+1:end);
end
x=struct('filename',filename);
global uidmap framemap
uidmap=[];
framemap=[];
gotframe=0;
frame=[];
while 1
  line=fgetl(fd);
  if line==-1
    break;
  end
  blanks=find(line==' ');
  if length(blanks)<1
    continue
  end
  path=line(1:blanks(1)-1);
  if ~gotframe && ~strcmp(path,'/pf/frame')
    continue;
  end
  if strcmp(path,'/pf/update')
    p=textscan(line,'%s %d %f %d %f %f %f %f %f %f %d %d %d');
    data=struct('frame',p{2},'time',p{3},'uid',p{4},'x',p{5},'y',p{6},'vx',p{7},'vy',p{8},'major',p{9},'minor',p{10},'groupid',p{11},'groupsize',p{12},'channel',p{13});
    x=copyfields(x,data);
  elseif strcmp(path,'/pf/body')
    p=textscan(line,'%s %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d');
    data=struct('frame',p{2},'uid',p{3},'x',p{4},'y',p{5},'ex',p{6},'ey',p{7},'spd',p{8},'espd',p{9},'heading',p{10},'eheading',p{11},'facing',p{12},'efacing',p{13},'legdiam',p{14},'sigmadiam',p{15},'sep',p{16},'sigmasep',p{17},'leftness',p{18},'visibility',p{19});
    x=copyfields(x,data);
  elseif strcmp(path,'/conductor/conx')
    p=textscan(line,'%s %q %q %q %d %d %f %f');
    data=struct('type',p{2},'subtype',p{3},'cid',p{4},'uid1',p{5},'uid2',p{6},'value',p{7},'time',p{8});
    uid1pos=getuidpos(data.uid1);
    uid2pos=getuidpos(data.uid2);
    data.subtype=strrep(data.subtype,'-','_');
    x.conx(framepos,uid1pos,uid2pos).(data.subtype)=data;
  elseif strcmp(path,'/conductor/attr')
    p=textscan(line,'%s %q %d %f %f');
    data=struct('type',p{2},'uid',p{3},'value',p{4},'time',p{5});
    uidpos=getuidpos(data.uid);
    x.attr(framepos,uidpos).(data.type)=data;
  elseif strcmp(path,'/pf/frame')
    p=textscan(line,'%s %d');
    frame=p{2};
    framepos=getframepos(frame);
    x.frame(framepos,1)=double(frame);
    if size(x.frame,2)>1
      keyboard
    end
    gotframe=1;
  else
    %    fprintf('unprocessed line=%s\n',line);
  end
end

function uidpos=getuidpos(uid)
global uidmap
uidpos=find(uid==uidmap,1);
if isempty(uidpos)
  uidmap=[uidmap,uid];
  uidpos=length(uidmap);
end

function framepos=getframepos(frame)
global framemap
framepos=find(frame==framemap,1);
if isempty(framepos)
  framemap=[framemap,frame];
  framepos=length(framemap);
end

function x=copyfields(x,data)
global framemap
if isfield(data,'frame')
  framepos=getframepos(data.frame);
else
  framepos=length(framemap);
end
if isfield(data,'uid')
  uidpos=getuidpos(data.uid);
  if isfield(data,'frame')
    data=rmfield(data,'frame');
  end
  fn=fieldnames(data);
  for i=1:length(fn)
    x.(fn{i})(framepos,uidpos)=data.(fn{i});
  end
  if isfield(data,'groupsize') && data.groupsize>1
    x.attr(framepos,uidpos).ingroup=struct('type','ingroup','uid',data.uid,'value',1,'time',0);
  end
end

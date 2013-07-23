% Check ID's of strips
s1=arduino_ip(1);
nstrip=6;
ledperstrip=160;
%nstrip=1;ledperstrip=300;
colnames={'red','green', 'blue','magenta','cyan','yellow','pinkish', 'white'};
col={[1 0 0],[0 1 0],[0 0 1],[1 0 1],[0 1 1], [1 1 0], [1 0.5 0.5], [1 1 1]};
clkpins=[48,46,44,42,36,34,32,40];
datapins=clkpins+1;
nphase=3;
phase=0;
cnt=0;
tic;
while true
  cmd=[];
  cmd=[cmd,setled(s1,-1,[0,0,0],0,1)];
  for k=1:nstrip
    offset=(k-1)*ledperstrip;
    cmd=[cmd,setled(s1,(phase+1:nphase:ledperstrip-2)+offset,col{k}*127,0,1)];
    first=mod(k-3,length(col))+1;
    last=mod(k+1,length(col))+1;
    cmd=[cmd,setled(s1,offset,127*col{first},0,1)];
    cmd=[cmd,setled(s1,offset+ledperstrip-1,127*col{last},0,1)];
    if mod(cnt,100)==02
      fprintf('Strip %d is %s (first=%s,last=%s).  Clk=pin %d, Data=pin %d\n', k, colnames{k},colnames{first},colnames{last},clkpins(k),datapins(k));
    end
  end
  fprintf('Writing cmd with %d bytes\n', length(cmd));
  awrite(s1,cmd);
  show(s1);
  if mod(cnt,2)==0
    sync(s1,10);
    fprintf('Update rate=%.2f/second\n',2/toc);
    tic;
  end
  phase=mod(phase+1,nphase);
  cnt=cnt+1;
end

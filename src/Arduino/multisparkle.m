s1=arduino_ip;
sync(s1);
getnumrcvd(s1);
tic;
nset=0;
nled=numled();
level=100;
nsent=0;
ncycles=50;
cols=[127 0 0
      0 127 0
      0 0 127
      127 127 0
      127 0 127
      0 127 127];
setled(s1,-1,[0,0,0],1);show(s1);
meanon=50;
pon=1;
numon=5;
poff=pon*numon/meanon;
delay=0.05;
curon=[];
initnow=now;
col=[level,0,0];
while true
  counter=syncsend(s1);   % Send request for sync, check for it after sending a data block
  fprintf('Sent sync req %d\n', counter);
  for k=1:20
    cmd=[];
    newc=[];
    for i=1:length(curon)
      if rand(1)<poff
        cmd=[cmd,setled(s1,curon(i),[0,0,0])];
      else
        newc=[newc,curon(i)];
      end
    end
    fprintf('T=%.2f ',(now-initnow)*24*60*60);
    if (length(curon)>length(newc))
      fprintf('Turned off %d ', length(curon)-length(newc));
    end
    curon=newc;
    for i=1:numon
      if rand(1)<pon
        sel=randi(nled)-1;
        curon=[curon,sel];
        %      cmd=[cmd,setled(s1,sel,cols(randi(size(cols,1)),:))];
        cmd=[cmd,setled(s1,sel,col)];
      end
      if (length(curon)>length(newc))
        fprintf('Turned on %d ', length(curon)-length(newc));
      end
    end
    fprintf('Have %d col=%d,%d,%d\n',length(curon),col);
    cmd=[cmd,'G'];
    awrite(s1,cmd);
    nsent=nsent+length(cmd);
    nset=nset+1;
    pause(delay);
    col=col+randi(3,1,3)-2;
    col(col>127)=127;
    col(col<0)=0;
    col=round(col*level/sum(col));
  end
  fprintf('Wait for sync response %d...',counter);
  ok=syncwait(s1,counter,3.0);  % Wait 1 second for sync
  if ~ok
    fprintf('Failed\n');
    break;
  end
  fprintf('OK\n');
  nsent=nsent+2;  % For sync()
end

nrcvd=getnumrcvd(s1);
nsent=nsent+1;  % For getnumrcvd() cmd
fprintf('Sent %d, Arduino received %d\n', nsent, nrcvd);
if nsent~=nrcvd
  fprintf('**** Dropped %d bytes\n', nsent-nrcvd);
end
% Turn off LEDs
setled(s1,-1,[0,0,0],1);
show(s1);

elapsed=toc;
fprintf('Changed LEDs %d times in %.2f seconds = %.2f changes/second\n', nset, elapsed, nset/elapsed);

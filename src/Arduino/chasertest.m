s1=arduino_ip;
sync(s1);
getnumrcvd(s1);
tic;
nset=0;
nled=numled();
level=127;
nsent=0;
delay=0.01;
nrpt=1;
for rpt=1:nrpt
fprintf('Repeat %d/%d Num Sent=%d\n',rpt,nrpt,nsent);
counter=-1;
for k=uint32(0):nled
  c=[];
  pos=1;
  for i=max(uint32(0),k-1):min(k,nled-1)
    if i==k 
      cmd=setled(s1,i,level*[1,0,0],0);
    elseif i==k-1
      cmd=setled(s1,i,0*[1,0,0],0);
    else
      continue;
    end
    c(pos:pos+length(cmd)-1)=cmd;
    pos=pos+length(cmd);
  end
  awrite(s1,c);
  show(s1,delay);
  nsent=nsent+length(c)+3;
  nset=nset+1;
  if mod(k,100)==0
    fprintf('sync send...');
    n1=now;
    counter=syncsend(s1);
    nsent=nsent+2;  % For sync()
  end
  if (mod(k,100)==50 || k==nled) && counter>0
    fprintf('wait...');
    n2=now;
    syncwait(s1,counter,5);
    n3=now;
    fprintf('%.0f,%.0f msec\n',[n2-n1,n3-n2]*24*3600*1e3);
    counter=-1;
  end
end
sync(s1);
nsent=nsent+2;  % For sync()
end

nrcvd=getnumrcvd(s1);
nsent=nsent+1;  % For getnumrcvd() cmd
fprintf('Sent %d, Arduino received %d\n', nsent, nrcvd);
if nsent~=nrcvd
  fprintf('**** Dropped %d bytes\n', nsent-nrcvd);
end

elapsed=toc;
fprintf('Changed LEDs %d times in %.2f seconds = %.2f changes/second\n', nset, elapsed, nset/elapsed);

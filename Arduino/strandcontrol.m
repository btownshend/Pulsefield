%s1=arduino_start(1);
s1=arduino_ip;
tic;
nset=0;
nled=160;
level=10;
red=setled(s1,0,level*[1,0,0],0);
green=setled(s1,0,level*[0,1,0],0);
fprintf('At start Arduino already received %d bytes\n', getnumrcvd(s1));
nsent=0;
delay=0.5;
for p=1:1
  for k=uint32(1):8
    c=zeros(1,nled*5,'uint8');
    pos=1;
    for i=uint32(0):nled-1
      if bitget(i,k)==1
        cmd=red;
      else
        cmd=green;
      end
      cmd(2)=i;
      c(pos:pos+length(cmd)-1)=cmd;
      pos=pos+length(cmd);
    end
    c=c(1:pos-1);
    awrite(s1,c);
    nsent=nsent+length(c);
    c=show(s1,delay);
    nsent=nsent+length(c);  % For sync()
    if ~sync(s1,delay+1)
      error('Sync failure');
    end
    nsent=nsent+2;
    nset=nset+1;
  end
  nrcvd=getnumrcvd(s1);
  nsent=nsent+1;  % For getnumrcvd() cmd
  fprintf('Iteration %d, sent %d, Arduino received %d\n', p, nsent, nrcvd);
  if nsent~=nrcvd
    fprintf('**** Dropped %d bytes\n', nsent-nrcvd);
  end
end
% Now clear them all
c=setled(s1,-1,[0,0,0],1);
c=show(s1);
sync(s1,2);
elapsed=toc;
fprintf('Changed all %d LEDs %d times in %.2f seconds = %.2f changes/second\n', nled, nset, elapsed, nset/elapsed);

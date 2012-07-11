s1=arduino_ip;
sync(s1);
getnumrcvd(s1);
tic;
nset=0;
nled=numled();
level=100;
red=setled(s1,0,level*[1,0,0],0);
off=setled(s1,0,level*[0,0,0],0);
nsent=0;
last=uint8(0);
delay=.1;
ncycles=50;
c=[red,off,'G','P',delay*1000];
cols=[127 0 0
      0 127 0
      0 0 127
      127 127 0
      127 0 127
      0 127 127];
setled(s1,-1,[0,0,0],1);show(s1);
while true
  for k=1:ncycles
    sel=randi(nled)-1;
    c(3)=floor(sel/256);
    c(2)=sel-c(3)*256;
    if k==ncycles-1
      c(4:6)=0;
    else
      c(4:6)=cols(randi(size(cols,1)),:);
    end
    c(9)=floor(last/256);
    c(8)=last-c(9)*256;
    fprintf('%3d ',c);fprintf('\n');
    last=sel;
    awrite(s1,c);
    nsent=nsent+length(c);
    nset=nset+1;
    %  pause(rand(1)/10);
  end
  fprintf('Waiting for sync...');
  [ok,counter]=sync(s1,delay*ncycles*5);
  if ~ok
    fprintf('Failed sync\n');
    break;
  end
  fprintf('%d done\n',counter);
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

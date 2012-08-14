% Check speed of full changes of LEDS
s1=arduino_ip;
sync(s1);
nled=numled();
level=127;
nsent=0;
delay=0.0;
color=zeros(nled,3);
% Pattern
color(:,1)=mod((1:size(color,1))*1,127);
color(:,2)=mod((1:size(color,1))*2,127);
color(:,3)=mod((1:size(color,1))*3,127);
counter=-1;
getnumrcvd(s1);   % Clear Arduino receipt count
nrpt=100;
tic;
for rpt=1:nrpt
  fprintf('Repeat %d/%d Num Sent=%d\n',rpt,nrpt,nsent);
  n1=now;
  c1=setallleds(s1,color,0);
  c2=show(s1,delay,0);
  [counter,c3]=syncsend(s1,0);
  c=[c1,c2];
  awrite(s1,c);
  nsent=nsent+length(c);
  fprintf('sent %d bytes, wait...',length(c));
  n2=now;
  sync(s1);
  % Lot of strange timing things here
  % Not sending a sync for a few frames results in SLOWER overall speed
  nsent=nsent+2;
  n3=now;
  fprintf('%.0f,%.0f msec\n',[n2-n1,n3-n2]*24*3600*1e3);

  % Shift the colors
  color=color([end,1:end-1],:);
  color=randi(127,size(color));
end
elapsed=toc;

nrcvd=getnumrcvd(s1);
nsent=nsent+1;  % For getnumrcvd() cmd
fprintf('Sent %d, Arduino received %d\n', nsent, nrcvd);
if nsent~=nrcvd
  fprintf('**** Dropped %d bytes\n', nsent-nrcvd);
end

fprintf('Changed LEDs %d times in %.2f seconds = %.2f changes/second\n', nrpt, elapsed, nrpt/elapsed);

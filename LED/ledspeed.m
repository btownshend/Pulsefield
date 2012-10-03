% Check speed of full changes of LEDS
noshow=0;   % Don't actually update display -- allows for faster stressing of ethernet
s1=arduino_ip;
sync(s1);
nled=960;
level=127;
nsent=0;
delay=0.0;
color=zeros(nled,3);
% Pattern
color(:,1)=mod((1:size(color,1))*1,2);
color(:,2)=mod((1:size(color,1))*2,2);
color(:,3)=mod((1:size(color,1))*3,2);
counter=-1;
getnumrcvd(s1);   % Clear Arduino receipt count
nrpt=1000000;
nmeasure=10;
for rpt=1:nrpt
  if mod(rpt-1,nmeasure)==0
    tic;
  end
  fprintf('Repeat %d/%d Num Sent=%d\n',rpt,nrpt,nsent);
  n1=now;
  c1=setallleds(s1,color,1);
  n2=now;
  if noshow
    c2=0;
  else
    c2=show(s1,delay);
  end
  n3=now;
  nsent=nsent+length(c1)+length(c2);
  fprintf('sent %d bytes, wait...',length(c1)+length(c2));
  nsent=nsent+2;
  fprintf('%.0f,%.0f msec\n',[n2-n1,n3-n2]*24*3600*1e3);

  % Shift the colors
  color=color([end,1:end-1],:);
%  color=randi(127,size(color));
  if mod(rpt,nmeasure)==0
    elapsed=toc;
    fprintf('Changed LEDs %d times in %.2f seconds = %.2f changes/second\n', nmeasure, elapsed, nmeasure/elapsed);
    %    pause(1);
  end
end

nrcvd=getnumrcvd(s1);
nsent=nsent+1;  % For getnumrcvd() cmd
fprintf('Sent %d, Arduino received %d\n', nsent, nrcvd);
if nsent~=nrcvd
  fprintf('**** Dropped %d bytes\n', nsent-nrcvd);
end


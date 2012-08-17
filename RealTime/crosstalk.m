% Crosstalk - measure LED crosstalk
function p=crosstalk(p,onval)

minmargin=50;  % Minimum margin between on-level and off-level to flag a LED->camera

% Measure signal with all LEDs off, even only, odd only
lsctl(p,'pause');

s1=arduino_ip();
if nargin<2
  onval=p.colors{1}*127;  % See output from levcheck
end
when=now;
for i=1:5
  setled(s1,-1,[0,0,0],1);
  if i==1|| i==5
    ;  % Already all off
  elseif i==2
    setled(s1,0:2:length(p.led)-1,onval,1);
  elseif i==3
    setled(s1,1:2:length(p.led)-1,onval,1);
  elseif i==4
    % All on
    setled(s1,-1,onval,1);
  end
  show(s1);
  sync(s1);
  pause(0.3);
  vis{i}=getvisible(p,'setleds',false,'stats',true,'usefrontend',false);
end
when=(when+now)/2;  % Middle of time window
offlev=(vis{1}.lev(:,:)+vis{5}.lev(:,:))/2;
onlev=vis{4}.lev(:,:);
neighlev(:,1:2:length(p.led))=vis{3}.lev(:,1:2:length(p.led));
neighlev(:,2:2:length(p.led))=vis{2}.lev(:,2:2:length(p.led));
halflev(:,1:2:length(p.led))=vis{2}.lev(:,1:2:length(p.led));
halflev(:,2:2:length(p.led))=vis{3}.lev(:,2:2:length(p.led));
neighlev=max(offlev,neighlev);  % In case neighbor shows up lower
margin=onlev-offlev;
% Set threshold at max of on-lev-margin and halfway between on and neighbor-on levels
thresh=max(onlev-minmargin,(onlev+neighlev)/2);

% Check for low margin LEDs
for j=1:length(p.camera)
  c=p.camera(j).pixcalib;
  lowmargin=[c.valid] & margin(j,:)<minmargin;
  if sum(lowmargin)>0
    fprintf('Low margin for camera %d to LEDs %s\n', j, shortlist(find(lowmargin)));
  end

% Don't actually disable low margin ones since we're using correlation instead now
%  for i=1:length(lowmargin)
%    c(i).inuse=c(i).valid & ~lowmargin(i);
%  end
%  p.camera(j).pixcalib=c;
end

setfig('crosstalk');
clf;
for k=1:length(p.camera)
  subplot(length(p.camera),2,k*2-1);
  plot(1:length(p.led),onlev(k,:),'g');
  hold on;
  plot(1:length(p.led),neighlev(k,:),'r');
  plot(1:length(p.led),offlev(k,:),'y');
  plot(1:length(p.led),halflev(k,:),'m');
  xlabel('LED');
  ylabel('Signal');
  legend('All LEDs on','Neighs only','All Off','Half On');
  title(sprintf('Camera %d signals',k));
  c=axis; c(3)=0;axis(c);

  subplot(length(p.camera),2,k*2);
  plot(1:length(p.led),onlev(k,:)-offlev(k,:),'g');
  hold on;
  plot(1:length(p.led),neighlev(k,:)-offlev(k,:),'r');
  xlabel('LED');
  ylabel('Signal');
  legend('LED-Bkgnd','Neighs crosstalk');
  title(sprintf('Camera %d crosstalk',k));
  c=axis; c(3)=-10;c(4)=260;axis(c);
end
pause(0.01);

ct=struct('onlev',onlev,'offlev',offlev,'neighlev',neighlev,'halflev',halflev,'thresh',thresh,'margin',margin,'when',when);
p.crosstalk=ct;
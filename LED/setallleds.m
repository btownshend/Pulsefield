% Turn on LEDs
% setallleds(s1,color,vgroup)
% s1-descriptor
% color(N,3) - colors for each of numled() leds
% vgroup - virtual LED group (0-normal use for PF, 1-unmapped, 2-entry left, 3-entry right)
function cmd=setallleds(s1,color,vgroup)
cmd=[];
if nargin<3
  vgroup=0;
end

if vgroup==0
%  posmap=[800+(29:-1:0),640+(159:-1:0),320+(159:-1:0),480+(0:159),0:159,160+(0:28),800+(159:-1:30),160+(30:159)];
  posmap=[800+(29:-1:0),640+(159:-1:0),320+(159:-1:0),480+(0:159),0:159,160+(0:28)];
elseif vgroup==1
  posmap=0:size(color,1)-1;
  %fprintf('Not mapping IDs\n');
elseif vgroup==2
  % Map index to left entry group (TODO-tune)
  posmap=[800+(159:-1:30)];
elseif vgroup==3
  % Map index to right entry group (TODO-tune)
  posmap=[160+(30:159)];
end

if size(color,1)~=length(posmap)
  fprintf('setallleds with %d colors instead of expected %d\n', size(color,1),length(posmap));
  return;
end

physleds=max(posmap)+1;
% Remap colors
mcolor=zeros(physleds,3,'uint8');
mcolor(posmap+1,:)= bitset(uint8(color),8);

nsent=0;
while nsent<size(mcolor,1)
  send=min(size(mcolor,1)-nsent,100);
  pcmd=zeros(1,4+3*send,'uint8');
  [cntr,sscmd]=syncsend(s1,1);
  pcmd(1)='F';
  pcmd(2)=bitand(nsent,255);
  pcmd(3)=bitshift(nsent,-8);
  pcmd(4)=send;
  ind=1:3:send*3-2;
  % Reorder as GRB
  pcmd(ind+4)=mcolor(nsent+1:nsent+send,2);
  pcmd(ind+5)=mcolor(nsent+1:nsent+send,1);
  pcmd(ind+6)=mcolor(nsent+1:nsent+send,3);
  awrite(s1,[pcmd]);
  cmd=[cmd,sscmd,pcmd];
  % Lot of strange timing things here
  % Not sending a sync for a few frames results in SLOWER overall speed
  % Probably overrunning Arduino's WD5100's receive buffer (2048 bytes) (see Arduino.app/../w5100.h)
  ok=syncwait(s1,cntr);
  if ~ok
    error('Failed sync wait');
  end
  nsent=nsent+send;
end

  
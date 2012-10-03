% Turn on LEDs
% setallleds(s1,color,vgroup)
% s1-descriptor
% color(N,3) - colors for each of numled() leds
% vgroup - virtual LED group (0-normal use for PF, 1-unmapped, 2-entry left, 3-entry right)
function [cmd,mcolor]=setallleds(s1,color,vgroup)
cmd=[];
if iscell(color)
  % color is cell array containing main, left, right
  if length(color)~=3 || nargin~=2
    error('Cell inputs to setallleds should have 3 elements for color (main, left, right)');
  end
  vg=[0,2,3];
  allpos=getposmap(1);
  firstled=min(allpos);
  physleds=max(allpos)-firstled+1;
  mcolor=128*ones(physleds,3,'uint8');
  for i=1:3
    posmap=getposmap(vg(i));
    if size(color{i},1)~=length(posmap)
      error('setallleds element %d with %d colors instead of expected %d\n', i, size(color{i},1),length(posmap));
    end
    mcolor(posmap-firstled+1,:)=bitset(uint8(color{i}),8);
  end
else  
  if nargin<3
    vgroup=0;
  end
  posmap=getposmap(vgroup);

  if size(color,1)~=length(posmap)
    fprintf('setallleds with %d colors instead of expected %d\n', size(color,1),length(posmap));
    return;
  end

  firstled=min(posmap);
  physleds=max(posmap)-firstled+1;
  % Remap colors
  mcolor=zeros(physleds,3,'uint8');
  mcolor(posmap-firstled+1,:)= bitset(uint8(color),8);
end

nsent=0;
pos=firstled;
cntr=[];
while nsent<size(mcolor,1)
  send=min(size(mcolor,1)-nsent,255);
  pcmd=zeros(1,4+3*send,'uint8');
  [cntr,sscmd]=syncsend(s1,1);
  pcmd(1)='F';
  % fprintf('Setting %d-%d\n',pos,pos+send-1);
  pcmd(2)=bitand(pos,255);
  pcmd(3)=bitshift(pos,-8);
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
  pos=pos+send;
end

  
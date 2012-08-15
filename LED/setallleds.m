% Turn on LEDs
% setallleds(s1,firstindex,color[],execute,unmapped)
% s1-descriptor
% color(N,3) - colors for each of numled() leds
% execute - true to send to Arduino now (default true)
% unmapped - true to NOT map to ordered LEDs (default false)
function cmd=setallleds(s1,color,execute,unmapped)
cmd=[];
if nargin<3
  execute=1;
end

% Map to correct physical led number
if nargin>=4 && unmapped
  fprintf('Unmapped\n');
  posmap=0:size(color,1)-1;
else
  if size(color,1)~=numled()
    fprintf('setallleds with %d colors instead of expected %d\n', size(color,1),numled());
    return;
  end
  posmap=[480+(159:-1:0),320+(0:50),0+(159:-1:0),160+(0:159)];
end

physleds=max(posmap)+1;
% Remap colors
mcolor=zeros(physleds,3,'uint8');
mcolor(posmap+1,:)= bitset(uint8(color),8);

nsent=0;
while nsent<size(mcolor,1)
  send=min(size(mcolor,1)-nsent,255);
  pcmd=zeros(1,4+3*send,'uint8');
  pcmd(1)='F';
  pcmd(2)=bitand(nsent,255);
  pcmd(3)=bitshift(nsent,-8);
  pcmd(4)=send;
  ind=1:3:send*3-2;
  % Reorder as GRB
  pcmd(ind+4)=mcolor(nsent+1:nsent+send,2);
  pcmd(ind+5)=mcolor(nsent+1:nsent+send,1);
  pcmd(ind+6)=mcolor(nsent+1:nsent+send,3);
  cmd=[cmd,pcmd];
  nsent=nsent+send;
end

if execute
  awrite(s1,cmd);
end
  
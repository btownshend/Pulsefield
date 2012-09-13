% Turn on LEDs
% setled(s1,index,color,execute,vgroup)
% s1-descriptor
% index-LED number(s) to turn on (-1 for all LEDs), a 2-element vector for a range, otherwise the LED numbers
% LED number has 0-origin
% vgroup - virtual LED group (0-normal use for PF, 1-unmapped, 2-entry left, 3-entry right)
% Remapped to order the strip/leds around the pulsefield methodically (unless arg5 is true, then no mapping is done)
function cmd=setled(s1,index,color,execute,vgroup)
if nargin<4
  execute=1;
end
if nargin<5
  vgroup=0;
end
if length(index)==0
  return;
end
if length(index)==1 && index==-1 && max(color)==0
  % All LEDs
  cmd=zeros(1,4,'uint8');
  cmd(1)='A';
  cmd(2)=color(1);
  cmd(3)=color(2);
  cmd(4)=color(3);
else
  if length(index)==1 && index==-1
    index=0:numled()-1;
  elseif length(index)==2
    index=index(1):index(2);
  end
  if vgroup==0
    % Map index (clockwise sequential) to correct led number
    posmap=[800+(29:-1:0),640+(159:-1:0),320+(159:-1:0),480+(0:159),0:159,160+(0:28)];
%    posmap=[480+(159:-1:0),320+(0:50),0+(159:-1:0),160+(0:159)];
    if max(index+1)>length(posmap) || min(index+1)<1
      error('Index out of range; should be 0..%d\n', length(posmap)-1);
    end
    index=sort(posmap(index+1));
  elseif vgroup==1
    %fprintf('Not mapping IDs\n');
  elseif vgroup==2
    % Map index to left entry group (TODO-tune)
    posmap=[800+(159:-1:30)];
    if max(index+1)>length(posmap) || min(index+1)<1
      error('Index out of range; should be 0..%d\n', length(posmap)-1);
    end
    index=sort(posmap(index+1));
  elseif vgroup==3
    % Map index to right entry group (TODO-tune)
    posmap=[160+(29:159)];
    if max(index+1)>length(posmap) || min(index+1)<1
      error('Index out of range; should be 0..%d\n', length(posmap)-1);
    end
    index=sort(posmap(index+1));
  end

  i=1;
  cmd=[];
  while i<=length(index)
    j=i;
    while j<length(index) && index(j+1)==index(j)+1
      j=j+1;
    end
    if j>i
      % Set a group
      pcmd=zeros(1,8,'uint8');
      pcmd(1)='R';
      pcmd(2)=bitand(index(i),255);
      pcmd(3)=bitshift(index(i),-8);
      pcmd(4)=bitand(index(j),255);
      pcmd(5)=bitshift(index(j),-8);
      pcmd(6)=color(1);
      pcmd(7)=color(2);
      pcmd(8)=color(3);
      %            fprintf('Set %d-%d\n',index(i),index(j));
    else
      % A single LED
      pcmd=zeros(1,6,'uint8');
      pcmd(1)='S';
      pcmd(2)=bitand(index(i),255);
      pcmd(3)=bitshift(index(i),-8);
      pcmd(4)=color(1);
      pcmd(5)=color(2);
      pcmd(6)=color(3);
      %      fprintf('Set %d\n',index(i));
    end
    cmd=[cmd,pcmd];
    i=j+1;
  end
end
if execute
  awrite(s1,cmd);
end


function cmd=show(s1,holdtime)
cmd=uint8('G');
if nargin>1
  while floor(holdtime*1000)>0
    cmd=[cmd,'P',uint8(holdtime*1000)];
    holdtime=holdtime-cmd(end)/1000;
  end
end
awrite(s1,cmd);


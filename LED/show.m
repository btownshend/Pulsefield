function cmd=show(s1,holdtime,execute)
if nargin<3
  execute=true;
end
cmd=uint8('G');
if nargin>1
  while floor(holdtime*1000)>0
    cmd=[cmd,'P',uint8(holdtime*1000)];
    holdtime=holdtime-cmd(end)/1000;
  end
end
if execute
  awrite(s1,cmd);
end


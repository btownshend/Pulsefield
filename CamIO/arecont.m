function p=arecont(id,half)
if nargin<2
  half=0;
end
[h,port]=getsubsysaddr(sprintf('CA%d',id),'reload',false);
if half
  res='half';
else
  res='full';
end
url=sprintf('http://%s/image?res=%s&quality=21&doublescan=0&x0=0&x1=9999&y0=0&y1=9999',h,res);
cmd=sprintf('DYLD_LIBRARY_PATH=/opt/local/lib;curl -s ''%s'' >/tmp/im.jpg', url)
p.captstart=now;
[s,r]=system(cmd);
if s~=0
  error('Failed execution of command: %s\nError: %s\n',cmd,r);
end  
p.captend=now;
p.im=imread('/tmp/im.jpg');
p.info=imfinfo('/tmp/im.jpg');

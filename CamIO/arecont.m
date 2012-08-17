function p=arecont(id)
[h,p]=getsubsysaddr(sprintf('CA%d',id),'reload',false);
url=sprintf('http://%s/image?res=full&quality=21&doublescan=0&x0=0&x1=9999&y0=0&y1=9999',h);
cmd=sprintf('curl -s ''%s'' >/tmp/im.jpg', url);
p.captstart=now;
system(cmd);
p.captend=now;
p.im=imread('/tmp/im.jpg');
p.info=imfinfo('/tmp/im.jpg');

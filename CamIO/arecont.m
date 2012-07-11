function p=arecont(id)
url=sprintf('http://192.168.0.%d/image?res=full&quality=21&doublescan=0&x0=0&x1=9999&y0=0&y1=9999',70+id);
cmd=sprintf('curl -s ''%s'' >/tmp/im.jpg', url);
p.captstart=now;
system(cmd);
p.captend=now;
p.im=imread('/tmp/im.jpg');
p.info=imfinfo('/tmp/im.jpg');

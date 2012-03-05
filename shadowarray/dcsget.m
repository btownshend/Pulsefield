function im=dcsget()
tic
t1=now;
system('curl -u demo: http://camera.local.tc.com/cgi-bin/video.jpg >/tmp/dcs.jpg');
t2=now;
toc
fprintf('%s - %s\n', datestr(t1), datestr(t2));
im=imread('/tmp/dcs.jpg');
figure(gcf);
imshow(im);

function [im,info]=dcsget(url)
if nargin<1
  url='http://192.168.0.70/cgi-bin/viewer/video.jpg?quality=5';
end
%tic
%t1=now;
%url='-u demo: http://camera.local.tc.com/cgi-bin/video.jpg';
system(sprintf('curl -s %s >/tmp/dcs.jpg', url));
%t2=now;
%toc
%fprintf('%s - %s\n', datestr(t1), datestr(t2));
im=imread('/tmp/dcs.jpg');
info=imfinfo('/tmp/dcs.jpg');
%figure(gcf);
%imshow(im);

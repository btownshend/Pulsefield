% Capture frames from ARECONT
cam=2;
h=sprintf('192.168.0.%d',cam+70);
url=sprintf('http://%s/image?res=full&quality=21&doublescan=0&x0=0&x1=9999&y0=0&y1=9999',h);
dircnt=1;
while true
  dirname=sprintf('C%d.%d',cam,dircnt);
  if ~exist(dirname,'dir')
    break;
  end
  dircnt=dircnt+1;
end
mkdir(dirname);
fprintf('Saving images to %s\n',dirname);
icnt=1;
while true
  fname=sprintf('%s/c%d-%d.jpg',dirname,cam,icnt);
  cmd=sprintf('DYLD_LIBRARY_PATH=/opt/local/lib;/opt/local/bin/curl ''%s'' >%s', url,fname);
%  cmd=sprintf('/usr/bin/curl ''%s'' >%s', url,fname);
  for i=5:-1:1
    pause(1);
    fprintf('%d...',i);
  end
  fprintf('GO\007\n');
  [s,r]=system(cmd);
  if s~=0
    error(['system(''',cmd,'''): failed; s=',num2str(s),', r=',r]);
  end
  im=imread(fname);
  imshow(im);figure(gcf);
  icnt=icnt+1;
end

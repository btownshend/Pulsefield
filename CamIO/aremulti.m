% Load simultaneously from multiple cameras
% id is an array of ids to load
% cam is type of camera (av10115 or av10115-half)
% result is a cell array of images
function im=aremulti(id,type,roi,plot)
if strcmp(type,'av10115')
  res='full';
elseif strcmp(type,'av10115-half')
  res='half';
else
  error('aremulti: Unsupported camera type: %s\n', type);
end
p.captstart=now;
cmd='';
for i=1:length(id)
  % TODO - would be faster to open connection once and then make HTTP requests without closing
  % or use MJPEG
  [h,p]=getsubsysaddr(sprintf('CA%d',id(i)));
  assert(~isempty(h));
  url=sprintf('http://%s/image?res=%s&quality=21&doublescan=1&ver=HTTP/1.1',h,res);
  if nargin<3 || isempty(roi)
    url=[url,sprintf('&x0=0&x1=9999&y0=0&y1=9999')];
  else
    if strcmp(res,'half')
      roi{i}=(roi{i}-1)*2+1;   % ROI sent to camera is still in full-size coordinate space
    end
    url=[url,sprintf('&x0=%d&x1=%d&y0=%d&y1=%d',roi{i}-1)];
  end
  cmd=[cmd,sprintf('curl -s ''%s'' >/tmp/im%d.jpg &', url,id(i))];
end
cmd=[cmd,' wait'];
system(cmd);
im=cell(1,length(id));
for i=1:length(id)
  fname=sprintf('/tmp/im%d.jpg',id(i));
  try
    im{i}=imread(fname);
  catch ex
    fprintf('Failed first attempt to open %s: %s\n', fname, ex.message);
    pause(0.1);
    im{i}=imread(fname);
  end
end
if nargin>=4 && plot
  setfig('aremulti');
  for i=1:length(id)
    subplot(2,ceil(length(id)/2),i)
    imshow(im{i});
  end
end
  
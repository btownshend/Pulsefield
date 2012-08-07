% Load simultaneously from multiple cameras
% id is an array of ids to load
% type is type of camera
% result is a cell array of images
function im=aremulti_jtcp(id,type,roi,plot)
if ~strcmp(type,'av10115')
  error('Unsupported camera type: %s\n', type);
end
p.captstart=now;
fd={};
for i=1:length(id)
  req=sprintf('GET /image?res=full&quality=21&doublescan=0');
  if nargin<2 || isempty(roi{i})
    req=[req,sprintf('&x0=0&x1=9999&y0=0&y1=9999')];
  else
    req=[req,sprintf('&x0=%d&x1=%d&y0=%d&y1=%d',roi{i}-1)];
  end
  req=[req,sprintf(' HTTP/1.1\r\n\r\n')];
  [h,p]=getsubsysaddr(sprintf('CA%d',id(i)));
  try
    fd{i}=jtcp('REQUEST',h,p,'SERIALIZE',false)
  catch me
    error('Failed open of camera %d: %s',id,me.message);
  end
  jtcp('write',fd{i}, uint8(req));
end
p.captend=now;
lbytes=zeros(1,length(id));
resp={};
for i=1:length(id)
  resp{i}=uint8('');
end
while true
  changed=0;
  for i=1:length(id)
    resp{i}=[resp{i},jtcp('read',fd{i})];
    fprintf('%6d ',length(resp{i}));
  end
  fprintf('\n');
  pause(1);
end
fprintf('Files completed after %.3f sec\n',(now-p.captend)*24*3600);
im={};
for i=1:length(id)
  jtcp('close',fd{i});
  fname=sprintf('/tmp/im%d.jpg',id(i));
  tmpfd=fopen(fname,'w');
  fwrite(tmpfd,resp);
  fclose(tmpfd);
  im{i}=imread(fname);
end
if nargin>=3 && plot
  setfig('aremulti');
  for i=1:length(id)
    subplot(2,ceil(length(id)/2),i)
    imshow(im{i});
  end
end
  
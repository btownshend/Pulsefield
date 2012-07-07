% Load simultaneously from multiple cameras
% id is an array of ids to load
% result is a cell array of images
function im=aremulti(id,roi,plot)

p.captstart=now;
cmd='';
for i=1:length(id)
  url=sprintf('http://192.168.0.%d/image?res=full&quality=21&doublescan=0',70+id(i));
  if nargin<2 || isempty(roi{i})
    url=[url,sprintf('&x0=0&x1=9999&y0=0&y1=9999')];
  else
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
if nargin>=3 && plot
  setfig('aremulti');
  for i=1:length(id)
    subplot(2,ceil(length(id)/2),i)
    imshow(im{i});
  end
end
  
% Manually indicate horizon
function p=locatehorizon(p,im)
if nargin<2
  im=aremulti([p.camera.id],p.camera(1).type);
end

for c=1:length(p.camera)
  setfig(sprintf('Horizon',c));clf;
  imshow(adapthisteq(rgb2gray(im{c}),'ClipLimit',.1));
  hold on;
  p.camera(c).horizon=[];
  p.camera(c).othercams=[];
  fprintf('Click on other visible cameras; press return when done: ');
  while true
    [x,y]=ginput(1);
    if isempty(x)
      break;
    end
    p.camera(c).othercams(end+1,:)=[x,y];
    p.camera(c).horizon(end+1,:)=[x,y];
    plot(x,y,'og');
  end
  fprintf('Click on other horizon points; press return when done: ');
  while true
    [x,y]=ginput(1);
    if isempty(x)
      if size(p.camera(c).horizon,1)>=4
        break;
      else
        fprintf('Need at least 4 horizon points -- only have %d so far\n',size(p.camera(c).horizon,1));
        continue;
      end
    end
    p.camera(c).horizon(end+1,:)=[x,y];
    plot(x,y,'or');
  end
end
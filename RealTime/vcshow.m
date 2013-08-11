% Show an entry from the viscache
function vcshow(p,camera,led)
cnt=1;
for c=camera
  for l=led
    subplot(length(camera),length(led),cnt);
    cnt=cnt+1;
vc=p.camera(c).viscache;
tlpos=vc.tlpos(l,:);
brpos=vc.brpos(l,:);
margin=4;
refim=vc.refim(tlpos(2)-margin:brpos(2)+margin,tlpos(1)-margin:brpos(1)+margin,:);
imshow(imresize(refim,10,'nearest'));
title(sprintf('Camera %d, LED %d', camera, l));
  end
end


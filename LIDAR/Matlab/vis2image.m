% Overlay a scan onto an image with given mapping
function im=vis2image(vis,im, winbounds,isbg)
xy=range2xy(vis.angle+pi/2,vis.range);
xy(:,1)=(xy(:,1)-winbounds(1))/(winbounds(2)-winbounds(1))*(size(im,2)-1)+1;
xy(:,2)=(winbounds(4)-xy(:,2))/(winbounds(4)-winbounds(3))*(size(im,1)-1)+1;
colors=[0 0 1
        0 1 0
        1 0 1
        1 1 0
        1 0 1
        0 1 1
        0.5 0.5 0.5];
if isa(im,'uint8')
  colors=colors*255;
end
if isbg
  % Draw as lines
  delta=[0,0;diff(xy)];
  nc=all(delta'==0);
  xy=xy(~nc,:); delta=delta(~nc,:);
  r=cumsum(sqrt(delta(:,1).^2+delta(:,2).^2));
  x=interp1(r,xy(:,1),0:max(r));
  y=interp1(r,xy(:,2),0:max(r));
  x=round(min(max(1,x),size(im,2)));
  y=round(min(max(1,y),size(im,1)));
  for k=1:length(x)
    im(y(k),x(k),1)=0;
  end
else
  xy=round(xy);
  xy(xy(:)<1)=1;
  xy(xy(:,2)>size(im,1),2)=size(im,1);
  xy(xy(:,1)>size(im,2),1)=size(im,2);
  if isfield(vis,'targets')
    col=vis.targets.class;
  else
    col=1;
  end
  col(col>size(colors,1))=size(colors,1);
  fuzz=1;
  for i=1:size(xy,1)
    if col(i)==0
      % Don't draw background
      continue;
    end
    for fx=-fuzz:fuzz
      if xy(i,1)+fx<1 || xy(i,1)+fx>size(im,2)
        continue;
      end
      for fy=-fuzz:fuzz
        if xy(i,2)+fy<1 || xy(i,2)+fy>size(im,1)
          continue;
        end
        im(xy(i,2)+fy,xy(i,1)+fx,:)=colors(col(i),:);
      end
    end
  end
end
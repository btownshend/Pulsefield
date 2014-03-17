% Overlay a scan onto an image with given mapping
function im=vis2image(vis,im, winbounds,isbg)
if isbg
  xy=range2xy(vis.angle,vis.range(1,:));
else
  xy=range2xy(vis.angle,vis.range);
end
xy(:,1)=(xy(:,1)-winbounds(1))/(winbounds(2)-winbounds(1))*(size(im,2)-1)+1;
xy(:,2)=(winbounds(4)-xy(:,2))/(winbounds(4)-winbounds(3))*(size(im,1)-1)+1;
colors=[0 0.2 0   % Background points
    	0 0 1   % Outside background
        0.2 0 0	% Noise
        1 0 1	% Target1
        1 1 0	% Target2
        1 0 1	% ...
        0 1 1
        0.5 0.5 0.5  % Any additional targets
       ];
bgline=[0.1 0.1 0.1];
if isa(im,'uint8')
  colors=uint8(colors*255);
  bgline=uint8(bgline*255);
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
  ind=sub2ind(size(im),y,x,ones(size(x)));
  skip=size(im,1)*size(im,2);  % Offset of each color plane
  for k=1:3
    im(ind)=im(ind)+bgline(k);
    ind=ind+skip;   % Advance to next color plane
  end
else
  xy=round(xy);
  fuzz=1;
  xy(xy(:)<1+fuzz)=1+fuzz;
  xy(xy(:,2)>size(im,1)-fuzz,2)=size(im,1)-fuzz;
  xy(xy(:,1)>size(im,2)-fuzz,1)=size(im,2)-fuzz;
  col=vis.class+1;
  col(col>size(colors,1))=size(colors,1);
  ind=sub2ind(size(im),xy(:,2),xy(:,1),ones(size(xy,1),1));
  skip2=size(im,1);
  skip3=size(im,1)*size(im,2);
  for k=1:3
    for fx=-fuzz:fuzz
      for fy=-fuzz:fuzz
        ind1=ind+fy+fx*skip2+(k-1)*skip3;
        im(ind1)=im(ind1)+colors(col,k);
      end
    end
  end
end
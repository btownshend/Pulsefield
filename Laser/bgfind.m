% Find targets in the background
bg=load('bg');
dist=bg(:,4);
dist(dist>6)=nan;
dtheta=(bg(2,3)-bg(1,3))*pi/180;
[x,y]=pol2cart(bg(:,3)*pi/180,dist);
pt=[-y,x];
obj=0;
maxobj=0
cornerError=20;		% Error in angle of corner in degrees
orienterror=30;		% Error in which way corner is aiming in degrees
maxsep=dist*dtheta*tand(45+orienterror);
for i=2:size(pt,1)
  dpt=pt(i)-pt(i-1);
  if any(isnan(pt(i,:)))
    obj(i)=0;
  elseif any(isnan(pt(i-1))) || norm(dpt)>maxsep(i)
    maxobj=maxobj+1;
    obj(i)=maxobj;
  else
    obj(i)=obj(i-1);
  end
end
for i=1:max(obj)
  % Remove end points
  obj(min(find(obj==i)))=0;
  obj(max(find(obj==i)))=0;
end

for i=1:max(obj)
  if sum(obj==i)<4
    obj(obj==i)=0;
    continue;
  end
  pts=pt(obj==i,:);

  sz=100*norm(pts(1,:)-pts(end,:));
  sep=sqrt(max(sum((pts(2:end,:)-pts(1:end-1,:)).^2,2)));
  fprintf('obj(%d) @ [%.2f,%.2f]: %d points, size=%.1fcm maxsep=%.1fcm ', i, mean(pts), size(pts,1),sz, sep*100);
  if any(isnan(mean(pts)))
    keyboard
  end
  if sz<10 || sz > 40
    fprintf('reject\n');
    obj(obj==i)=0;
  else
    [corners{i},minerr,angle, orient]=cornerfit(pts);
    fprintf('RMS err=%.1fcm angle=%.0f orient=%.0f ',minerr*100, angle, orient);
    if minerr>0.005 || abs(angle-90)>cornerError || abs(orient)>orienterror
      fprintf('reject\n');
      obj(obj==i)=0;
    else
      fprintf('accept\n');
    end
  end
end

setfig('bg');clf;
plot(0,0,'o');
hold on;
plot(pt(:,1)*100,pt(:,2)*100,'.k','MarkerSize',2);
for i=unique(obj)
  if i==0 || isnan(i)
    continue;
  end
  ptmp=pt;
  ptmp(obj~=i,:)=nan;
  h=plot(ptmp(:,1)*100,ptmp(:,2)*100,'o');
  c=corners{i};
  plot(c(:,1)*100,c(:,2)*100,'-','LineWidth',3,'Color',get(h,'Color'));
end
axis equal
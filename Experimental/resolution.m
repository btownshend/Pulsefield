pixperradian=3800/(100*pi/180);
shift=.001;   % Test at 1mm movements
nc=6;
width=7;
length=10;
cpos=zeros(6,2);
layout=4;
barwidth=27/39.37;
if layout==1
  cpos([1,5],1)=0;
  cpos([2,6],1)=cpos([1,5],1)+barwidth;
  cpos(5:6,2)=width;
  cpos(3,2)=(width-barwidth)/2;
  cpos(4,2)=(width+barwidth)/2;
elseif layout==2
  cpos(5,1)=(length+barwidth)/2;
  cpos(6,1)=(length-barwidth)/2;
  cpos([1,3],2)=(width-barwidth)/2;
  cpos([2,4],2)=(width+barwidth)/2;
  cpos([3,4],1)=length;
elseif layout==3
  cpos([1,2],1)=length;
  cpos(1,2)=(width+barwidth)/2;
  cpos(2,2)=(width-barwidth)/2;
  cpos([4,6],1)=cpos([3,5],1)+barwidth;
  cpos([5,6],2)=width;
elseif layout==4
  cpos(1,1)=barwidth/sqrt(2);
  cpos(2,2)=barwidth/sqrt(2);
  cpos(3,2)=(width-barwidth)/2;
  cpos(4,2)=(width+barwidth)/2;
  cpos([5,6],2)=width;
  cpos(5,1)=barwidth/sqrt(2);
  cpos(6,2)=width-barwidth/sqrt(2);
end

ndir=60;
step=0.1;
r=zeros(length/step+1,width/step+1,ndir);
for x=0:step:length
  i=round((x/step)+1);
  for y=0:step:width
    j=round((y/step)+1);
    for c=1:nc
      v=[x-cpos(c,1),y-cpos(c,2)];
      if v(1)<0 && v(2)>0 && v(1)/v(2)*(width-cpos(c,2))+cpos(c,1)<0
        continue;
      end
      if v(1)<0 && v(2)<0 && -v(1)/v(2)*cpos(c,2)+cpos(c,1)<0
        continue;
      end
      theta=atan(v(2)/v(1));
      %      if abs(theta)>60*pi/180
      %        continue;
      %      end
      for idir=1:ndir
        dir=(idir-1)/(ndir-1)*pi;
        dth=(atan((v(2)+shift*sin(dir))/(v(1)+shift*cos(dir)))-theta);
        r(i,j,idir)=r(i,j,idir)+dth.^2;
      end
    end
  end
end

minr=min(r,[],3);
mmperpixel=1./(sqrt(minr)*pixperradian);

setfig('resolution');clf;
pcolor(0:step:length,0:step:width,mmperpixel');
shading interp
axis equal
caxis([0,20]);
hold on;
plot(cpos(1:nc,1),cpos(1:nc,2),'xr');
title('mm/pixel');
colorbar;
for x=1:2:length
  for y=1:5/3:width
    val=mmperpixel(round(x/step)+1,round(y/step)+1);
    text(x,y,sprintf('%.1f',val),'HorizontalAlignment','center','VerticalAlignment','middle');
  end
end

% Show rays to a person
ploc=[5,4];
pdiam=0.3;
ax=axis;
col='rgmcyk';
for c=1:nc
  mray=ploc-cpos(c,:);
  perp=[mray(2),-mray(1)]/norm(mray);
  ray(1,:)=ploc+perp*pdiam/2-cpos(c,:);
  ray(2,:)=ploc-perp*pdiam/2-cpos(c,:);
  for i=1:2
    plot(cpos(c,1)+[0,ray(i,1)]*10,cpos(c,2)+[0,ray(i,2)]*10,col(c));
  end
end
axis(ax);

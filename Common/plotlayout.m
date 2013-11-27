function plotlayout(lp,newfig)
if isfield(lp,'layout')
  l=lp.layout;
  p=lp;
else
  l=lp;
  p=[];
end
if nargin<2
  newfig=1;
end
if newfig
  setfig('plotlayout');
  clf;
end
hold on;
plot(0,0,'x');

title(sprintf('Layout with %d cameras, %d LEDs',size(l.cpos,1),size(l.lpos,1)));
if ~isempty(l.lpos)
for i=1:size(l.lpos,1)
  h(1)=plot(l.lpos(:,1),l.lpos(:,2),'xm');
  plot(l.lpos(l.outsider,1),l.lpos(l.outsider,2),'xr');   % Plot outsiders in red
end
for k=1:50:size(l.lpos,1)
  text(l.lpos(k,1),l.lpos(k,2),sprintf('L%d',k));
  plot(l.lpos(k,1),l.lpos(k,2),'xk');
end
text(l.lpos(end,1),l.lpos(end,2),sprintf('L%d',size(l.lpos,1)));
end

labels{1}='LEDs';
for i=1:size(l.cpos,1)
  h(2)=plot(l.cpos(i,1),l.cpos(i,2),'ko');
  plot(0.5*[0,l.cdir(i,1)]+l.cpos(i,1),0.5*[0,l.cdir(i,2)]+l.cpos(i,2),'r-');
  text(l.cpos(i,1)-l.cdir(i,1)*0.2,l.cpos(i,2)-l.cdir(i,2)*0.2,sprintf('C%d',i),'HorizontalAlignment','center','VerticalAlignment','middle');
end
labels{2}='Cameras';

if ~isempty(l.active)
  h(3)=plot([l.active(:,1);l.active(1,1)],[l.active(:,2);l.active(1,2)],'c');
end
labels{3}='Active';

if ~isempty(l.entry)
  h(4)=plot(l.entry(1),l.entry(2),'r*');
end
labels{4}='Entry';

if ~isempty(p) && isfield(p,'rays') && ~isempty(p.rays)
  r=p.rays;
  for i=1:size(r.raylines,1)
    rr=pix2m(r.imap,r.raylines{i,1}(1:20,:));
    plot(rr(:,1),rr(:,2),'g');
    rr=pix2m(r.imap,r.raylines{i,end}(1:20,:));
    plot(rr(:,1),rr(:,2),'r');
  end
end
if ~isempty(p) && ~isempty(p.camera)
  radius=mean(sqrt(p.layout.cpos(:,1).^2+p.layout.cpos(:,2).^2));
  for i=1:length(p.camera)
    c=p.camera(i);
    if isfield(c,'othercams')
      col='rgbcymk';
      for j=1:size(c.othercams,1)
        angle=-c.anglemap(round(c.othercams(j,1)));
        dir=[];
        [dir(1),dir(2)]=pol2cart(cart2pol(p.layout.cdir(i,1),p.layout.cdir(i,2))+angle,1);
        path=[];
        for k=1:2
          path(:,k)=p.layout.cpos(i,k)+dir(k)*(0:.01:10);
        end
        path=path(1:find(path(:,1).^2+path(:,2).^2 < (1*radius)^2,1,'last'),:);
        plot(path(:,1),path(:,2),[col(i),':']);
      end
    end
  end
end


%legend(h,labels);
axis equal

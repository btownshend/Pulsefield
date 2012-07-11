% calcvisible - calculate visible LEDs given layout and targets
function v=calcvisible(p,layout,rays,tgts,doplot)
if nargin<5
  doplot=false;
end
cpos=layout.cpos;
cdir=layout.cdir;
lpos=layout.lpos;
tpos=tgts.tpos;
tgtdiam=tgts.tgtdiam;

if doplot
  % Draw layout
  setfig('calcvisible');
  clf;
  hold on;
  axis equal
  axis off
  col=1-0.3*[0 1 0
             1 0 0
             0 0 1
             1 0 1
             1 1 0
             0 1 1
             0.5 1 1
             1 0.5 1
             1 1 0.5
             0.5 0.5 1
             0.5 1 0.5
             1 0.5 0.5
            ];
end

for l=1:size(lpos,1)
  for c=1:size(cpos,1)
    vdir=lpos(l,:)-cpos(c,:);
    angle=acos(dot(vdir,cdir(c,:))/norm(vdir)/norm(cdir(c,:)));
    
    % npts=100;
    %    ray=[];
    %    ray(:,1)=(0:npts)/npts*(lpos(l,1)-cpos(c,1))+cpos(c,1);
    %    ray(:,2)=(0:npts)/npts*(lpos(l,2)-cpos(c,2))+cpos(c,2);

    if angle>p.camera(c).fov/2
      % Not in FOV, unknown visibility
      v(c,l)=nan;
    else
      ray=pix2m(rays.imap,rays.raylines{c,l});
      v(c,l)=1;
      for t=1:size(tpos,1)
        d=(ray(:,1)-tpos(t,1)).^2+(ray(:,2)-tpos(t,2)).^2;
        strikept=min(find(d<(tgtdiam(t)/2).^2));
        if ~isempty(strikept)
          % Blocked view
          v(c,l)=0;
          ray=ray(1:strikept,:);	% only keep first part of ray for subsequent target hunting
        end
      end
      if doplot && v(c,l)==0
        % Plot remaining part of ray (from camera to first hit target)
        plot(ray(:,1),ray(:,2),'Color',col(c,:));
      end
    end
  end
end

fprintf('Percent of LEDs visible = %.1f%%\n', 100*sum(v(:)==1)/sum(isfinite(v(:))));

if doplot
  % Plot layout
  plot(layout.lpos(:,1),layout.lpos(:,2),'.','MarkerSize',1);
  plot(layout.cpos(:,1),layout.cpos(:,2),'og');
  viscircles(tgts.tpos,tgts.tgtdiam/2,'LineWidth',0.5);

  % Plot FOV
  for i=1:size(layout.cpos,1)
    ca(1)=atan2(layout.cdir(i,2),layout.cdir(i,1))+p.camera(i).fov/2;
    ca(2)=atan2(layout.cdir(i,2),layout.cdir(i,1))-p.camera(i).fov/2;
    for j=1:2
      endpos=layout.cpos(i,:)+0.1*[cos(ca(j)),sin(ca(j))];
      plot([layout.cpos(i,1),endpos(1)],[layout.cpos(i,2),endpos(2)],'k');
    end
  end
  title('calcvisible');
end
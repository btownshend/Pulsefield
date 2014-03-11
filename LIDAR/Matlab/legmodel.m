% Model x,y locations as a pair leg and return struct holding:
%   c1 - center of leg1
%   c2 - center of leg2
%   radius - radius
%   err - mean square error
% Assume scan origin is 0,0
function legs=legmodel(vis,class,varargin)
  defaults=struct('maxlegdiam',0.3,...   % Maximum leg diameter
                  'minlegdiam',0.1,...   % Minimum
                  'maxlegsep',0.3,...
                  'debug',false...
                  );
  args=processargs(defaults,varargin);

  r=(args.minlegdiam+args.maxlegdiam)/4;

  if sum(vis.class==class)==0
    legs=struct('c1',[nan,nan],'c2',[nan,nan],'radius',r,'err',0);
    return;
  end
  xy=vis.xy(vis.class==class,:);
  shadowed=vis.shadowed(vis.class==class,:);
  leg=vis.leg(vis.class==class,:);
  
  npts=size(xy,1);
  c1=mean(xy(1:floor(npts/2),:),1);
  c2=mean(xy(ceil((npts+1)/2):end,:),1);
  delta=c2-c1;
  move=delta/norm(delta)*args.maxlegsep/2
  %  c1=c1-move
  %  c2=c2+move
  x0=[r,c1,c2];
  if args.debug
    options=optimset('Display','final');
  else
    options=optimset('Display','none');
  end
  x=fminsearch(@(x) calcerror(x(1),x(2:3),x(4:5),args,xy,shadowed,leg),x0,options);

  r=x(1);
  c1=x(2:3);
  c2=x(4:5);
  [err,a1]=calcerror(x(1),x(2:3),x(4:5),args,xy,shadowed,leg);
  %  if args.debug
  %    plotresults(r,c1,c2,args,xy,shadowed);
  %  end

  ass=ones(1,size(xy,1));
  ass(~a1)=2;
  legs=struct('c1',c1,'c2',c2,'radius',r,'err',sqrt(err),'assignment',ass);
end

function [err,assign1]=calcerror(r,c1,c2,args,xy,shadowed,leg)
  d1=sqrt((xy(:,1)-c1(1)).^2+(xy(:,2)-c1(2)).^2);
  d2=sqrt((xy(:,1)-c2(1)).^2+(xy(:,2)-c2(2)).^2);
  if all(leg)==0
    assign1=(d1-r).^2<(d2-r).^2;
  else
    assign1=leg==1;
  end
  err(assign1)=(d1(assign1)-r).^2;
  err(~assign1)=(d2(~assign1)-r).^2;
  penalty=zeros(1,4);

  if norm(c1-c2)<r*2
    % Penalty for overlapping legs
    penalty(1)=(norm(c1-c2)-2*r)^2;
  elseif norm(c1-c2)>r*2+args.maxlegsep
    % Penalty for too far apart
    penalty(2)=(norm(c1-c2)-(2*r+args.maxlegsep))^2;
  end

  if r>args.maxlegdiam/2
    % Too big
    penalty(3)=(r-args.maxlegdiam/2)^2;
  elseif r<args.minlegdiam/2
    % Too small
    penalty(4)=(r-args.minlegdiam/2)^2;
  end

  if ~shadowed(1,1)
    % Not shadowed on left, make sure edge of circle is not too big
    edist=line2pt([0,0],xy(1,:),c1);
    penalty(5)=(r-edist)^2;
  end
  if ~shadowed(end,2)
    % Not shadowed on right, make sure edge of circle is not too big
    edist=line2pt([0,0],xy(end,:),c2);
    penalty(6)=(r-edist)^2;
  end
  if args.debug
    plotresults(r,c1,c2,args,xy,shadowed);
    title(sprintf('err=%f,penalty=[%s]',sqrt(mean(err)),sprintf('%f ',sqrt(penalty))));
    pause(0.1);
  end
  
  err=mean([err,penalty]);
end


function plotresults(r,c1,c2,args,xy,shadowed)
  setfig('legmodel');clf;
  plot(xy(:,1),xy(:,2),'x');
  hold on;
  plot(c1(1),c1(2),'o');
  plot(c2(1),c2(2),'o');
  angle=-pi:.01:pi;
  [x,y]=pol2cart(angle,r);
  x1=x+c1(1);
  y1=y+c1(2);
  plot(x1,y1);
  x2=x+c2(1);
  y2=y+c2(2);
  plot(x2,y2);
  axis equal
end
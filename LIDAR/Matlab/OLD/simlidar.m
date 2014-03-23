% Simulate a LIDAR scan
% Usage: simlidar(targets,options)
% 	targets(:,2) - (x,y) positions of targets
% Options:
% 	resolution - angular resolution in degrees
function vis=simlidar(truth,varargin)
defaults=struct('resolution',.5,'sensorscan',[-95,95],'roomsize',[-4,-0.1,4,6.9],'targetsize',0.2,'sensordir',[0,1],'when',now);
args=processargs(defaults,varargin);

dist=[];
roomsize=args.roomsize;
roompolygon=[roomsize(1),roomsize(2),
             roomsize(3),roomsize(2),
             roomsize(3),roomsize(4),
             roomsize(1),roomsize(4),
             roomsize(1),roomsize(1)];
            
range=[]; reflect=[];
angles=(args.sensorscan(1):args.resolution:args.sensorscan(2))*pi/180;
for i=1:length(angles)
  angle=angles(i);
  [scandir(1),scandir(2)]=pol2cart(angle+cart2pol(args.sensordir(1),args.sensordir(2)),1);
  %  fprintf('angle=%.1f deg, scandir=(%.1f,%.1f)\n', angle*180/pi, scandir);
  dist=inf;
  for k=1:size(roompolygon,1)-1
    d=rayline([0,0],scandir,roompolygon(k,:),roompolygon(k+1,:));
    if d>0
      dist=min(dist,d);
    end
  end
  for k=1:size(truth.targets,1)
    d=linecircle([0,0],scandir,truth.targets(k,:),args.targetsize/2);
    dist=min(dist,d);
  end
  range(1,1,i)=dist;
  reflect(1,1,i)=1;
end
vis=struct('cframe',1,'echo',[1 0 0 0 0],'nmeasure',size(range,3),'angle',angles,'range',range,'frame',1,'acquired',args.when,'reflect',reflect,'when',args.when,'whenrcvd',args.when,'truth',truth);

function d=rayline(p1,p2,p3,p4)
x1=p1(1);y1=p1(2);
x2=p2(1);y2=p2(2);
x3=p3(1);y3=p3(2);
x4=p4(1);y4=p4(2);
denom=(x1-x2)*(y3-y4)-(y1-y2)*(x3-x4);
x=((x1*y2-y1*x2)*(x3-x4)-(x1-x2)*(x3*y4-y3*x4))/denom;
y=((x1*y2-y1*x2)*(y3-y4)-(y1-y2)*(x3*y4-y3*x4))/denom;
d=norm([x-x1,y-y1]);
if dot([x-x1,y-y1],[x2-x1,y2-y1])<0
  d=-d;
end
%fprintf('rayline((%.1f,%.1f)->(%.1f,%.1f), (%.1f,%.1f)-(%.1f,%.1f) = (%.1f,%.1f) d=%f\n',p1,p2-p1,p3,p4,x,y,d);


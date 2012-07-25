% updatetargets - update position of each target
function tgts=updatetargets(p,tgts)
% Determine if it stops/starts walking
ntgts=size(tgts,1);
start=tgts.speed>0 & rand(ntgts,1)<p.simul.dt/p.simul.meanstoptime;
stop=tgts.speed<0.1 & rand(ntgts,1)<p.simul.dt/p.simul.meanwalkingtime;
tgts.speed(stop)=0;
tgts.speed(start)=p.simul.speed;
% Change in heading (even if stopped)
tgts.heading=tgts.heading+randn(ntgts,1)*p.simul.stddirchange*p.simul.dt*pi/180;
% Calculate differential
hd=[cos(tgts.heading),sin(tgts.heading)];
dpos=[cos(tgts.heading).*tgts.speed,sin(tgts.heading).*tgts.speed]*p.simul.dt;
% Update
tgts.tpos=tgts.tpos+dpos;
% Find position of leading edge (front) of each target
% Check if any are outside layout
for i=1:2
  front(:,i)=tgts.tpos(:,i)+hd(:,i).*tgts.tgtdiam'/2;
end
outside=~inpolygon(front(:,1),front(:,2),np.layout.active(:,1),p.layout.active(:,2));
if sum(outside)>0
  fprintf('Bouncing %d targets off walls\n', sum(outside));
  tgts.tpos(outside,:)=tgts.tpos(outside,:)-dpos(outside,:);
  tgts.heading(outside,:)=mod(tgts.heading(outside,:)+pi,2*pi);
end

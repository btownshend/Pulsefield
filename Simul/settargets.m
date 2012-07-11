% settargets - choose some targets
function tgts=settargets(p,layout,ntgt)
fprintf('Targets are %.1f-%.1f cm in diameter\n', 100*[p.analysisparams.mintgtdiam,p.analysisparams.maxtgtdiam]);
mincdist=1;	% Minimum distance between cameras and targets


% Choose target positions
% Eliminate ones too close to camera or each other
fprintf('Choosing targets\n');
tpos=[];
tgtdiam=[];
minactive=min(layout.active);
maxactive=max(layout.active);
for i=1:ntgt;
  tgtdiam(i)=rand(1,1)*(p.analysisparams.maxtgtdiam-p.analysisparams.mintgtdiam)+p.analysisparams.mintgtdiam;
  % Find a legal position
  ok=0;
  while ~ok
    % Pick any point inside active bounding box
    tpos(i,:)=(maxactive-minactive-[tgtdiam(i),tgtdiam(i)]).*rand(1,2)+minactive+tgtdiam(i)*[1/2,1/2];
    ok=1;
    % Check if overlapping any existing target
    for j=1:i-1
      if norm(tpos(i,:)-tpos(j,:)) < (tgtdiam(i)+tgtdiam(j))/2
        ok=0;
%        fprintf('(%.1f,%.f) too close to target %d\n', tpos(i,:),j);
        break;
      end
    end
    
    % Check if inside polygon by casting a ray into the positive x direction and counting crossings
    if ~inpoly(layout.active,tpos(i,:))
      % Not inside
%      fprintf('(%.1f,%.f) not inside active area (ncross=%d)\n', tpos(i,:),ncross);
      ok=0;
    end
    
    % Check distance from each segment in active and discard if within tgtdiam
    for j=1:size(layout.active,1)
      j2=mod(j,size(layout.active,1))+1;
      delta=layout.active(j2,:)-layout.active(j,:);
      v=tpos(i,:)-layout.active(j,:);
      cosangle=dot(v,delta)/norm(v)/norm(delta);
      relpos=norm(v)/norm(delta)*cosangle;
      relpos=min(1,max(0,relpos));
      dist=norm(delta*relpos+layout.active(j,:)-tpos(i,:));
      if dist<tgtdiam/2
        fprintf('(%.1f,%.f) too close to perimeter\n', tpos(i,:));
        ok=0;
        break;
      end
    end
    if ok
      fprintf('(%.1f,%.f) ok\n', tpos(i,:));
    end
  end
end
fprintf('done\n');
tgts.tpos=tpos;
ntgts=size(tpos,1);
tgts.tgtdiam=tgtdiam;
tgts.heading=rand(ntgts,1)*2*pi;
tgts.speed=p.simul.speed*ones(ntgts,1);
% Some are stopped
tgts.speed(rand(ntgts,1)>p.simul.meanwalkingtime/(p.simul.meanstoptime+p.simul.meanwalkingtime))=0;

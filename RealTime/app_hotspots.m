% Hotspots app
function info=app_hotspots(info,op)
if ~isfield(info,'hotspots')|| strcmp(op,'start')
  nhotspots=10;
  info.hotspots.radius=1.2;
  info.hotspots.pos=(rand(nhotspots,2)*2-1)*2.3;   % TODO - make this depend on layout, check if inside, none too close together
  setfig('hotspots');
  clf;
  plotlayout(info.p.layout,0);
  hold on;
  plot(info.hotspots.pos(:,1),info.hotspots.pos(:,2),'*');
end
if ~strcmp(op,'update')
  return;
end
for i=1:length(info.updates)
  id=info.updates(i);
  ids=[info.snap.hypo.id];
  previds=[info.prevsnap.hypo.id];
  oldpos=info.snap.hypo(ids==id).pos;
  newpos=info.prevsnap.hypo(previds==id).pos;
  for j=1:size(info.hotspots.pos,1)
    dist=norm(newpos-info.hotspots.pos(j,:));
    %fprintf('dist(%d,%d)=%.2f\n',i,j,dist);
    if dist<=info.hotspots.radius
      pdist=norm(oldpos-info.hotspots.pos(j,:));
      if pdist>info.hotspots.radius
        % Just moved into place
        pitch=j+35;   % Drum kit goes from 35 to 81
        oscmsgout('MAX','/pf/pass/playmidinote',{int32(info.snap.hypo(i).id),int32(pitch),int32(info.velocity),int32(info.duration),10});
      end
    end
  end
end
    

function plotworld(p)

clf;
plot3([0,0],[0,0],[0,1],'r-');
hold on;
text(0,0,1,'Z');
plot3([0,0],[0,1],[0,0],'r-');
text(0,1,0,'Y');
plot3([0,1],[0,0],[0,0],'r-');
text(1,0,0,'X');

for c=1:length(p.camera)
  cam=p.camera(c);
  r=cam.extcal.extrinsic;
  campos=cw(cam.extcal,[0;0;0]);
  fprintf('Camera Position: [%.4f, %.4f, %.4f] in world\n', campos);
  plot3(campos(1),campos(2),campos(3),'gx');
  %  plot3([campos(1),campos(1)],[campos(2),campos(2)],[campos(3),0],'b');
  %  plot3([campos(1),campos(1)],[campos(2),0],[0,0],'g');
  % plot3([campos(1),0],[0,0],[0,0],'r');
  text(campos(1),campos(2),campos(3),sprintf('C%d',cam.id));
  
  % Plot camera box
  dist=0.15;vfov=90;hfov=130;
  xwidth=tand(hfov/2)*dist;
  ywidth=tand(vfov/2)*dist;
  positions={[xwidth;ywidth;dist],[-xwidth;ywidth;dist],[-xwidth;-ywidth;dist],[xwidth;-ywidth;dist],[xwidth;ywidth;dist]};
  for i=1:length(positions)
    p2=cw(cam.extcal,positions{i});
    plot3([campos(1),p2(1)],[campos(2),p2(2)],[campos(3),p2(3)],'b');
    if i>1
      plot3([pp(1),p2(1)],[pp(2),p2(2)],[pp(3),p2(3)],'b');
    end
    pp=p2;
  end
  up=cw(cam.extcal,[0,-dist,0]');
  plot3([campos(1),up(1)],[campos(2),up(2)],[campos(3),up(3)],'g');
end
box on;
axis vis3d;
axis equal;

% Add room outline and set axes to slightly larger than room
extra=0.5;
plot3(p.layout.active(:,1),p.layout.active(:,2),0*p.layout.active(:,1),'c');
axis([min(p.layout.active(:,1))-extra,max(p.layout.active(:,1))+extra,min(p.layout.active(:,2))-extra,max(p.layout.active(:,2))+extra,0,1.5]);

function wpts=cw(extcal,pts)
wpts=extcal.Rcw*pts;
for i=1:size(extcal.Tcw)
  wpts(i,:)=wpts(i,:)+extcal.Tcw(i);
end


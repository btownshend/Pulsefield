function plotworld(p,world)
if nargin<2
  if isfield(p,'calibration')
    world=struct('R',p.calibration.Rgw,'T',p.calibration.Tgw)
    fprintf('Displaying world coordinates based on last extrinsic calibration\n');
  else
    world=struct('R',eye(3),'T',zeros(3,1));
    fprintf('Displaying chessboard coordinates\n');
  end
end
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
  position=cam2grid(r,[0;0;0]);
  campos=tw(world,position);
  fprintf('Camera Position: [%.4f,%.4f,%.4f] in grid, [%.4f, %.4f, %.4f] in world\n', position, campos);
  grid=tw(world,r.Xgrid);
  plot3(grid(1,:),grid(2,:),grid(3,:),'.');
  
  for i=[1,p.calibration.target.nX+1,size(grid,2)-p.calibration.target.nY,size(grid,2)]
    fprintf('Grid Corner: [%.4f,%.4f,%.4f] in grid, [%.4f, %.4f, %.4f] in world\n', r.Xgrid(:,i), grid(:,i));
  end
  plot3(campos(1),campos(2),campos(3),'gx');
  plot3([campos(1),campos(1)],[campos(2),campos(2)],[campos(3),0],'b');
  plot3([campos(1),campos(1)],[campos(2),0],[0,0],'g');
  plot3([campos(1),0],[0,0],[0,0],'r');
  text(campos(1),campos(2),campos(3),sprintf('C%d',cam.id));
  
  % Plot camera box
  dist=0.2;vfov=90;hfov=130;
  xwidth=tand(hfov/2)*dist;
  ywidth=tand(vfov/2)*dist;
  positions={[xwidth;ywidth;dist],[-xwidth;ywidth;dist],[-xwidth;-ywidth;dist],[xwidth;-ywidth;dist],[xwidth;ywidth;dist]};
  for i=1:length(positions)
    p2=tw(world,cam2grid(r,positions{i}));
    plot3([campos(1),p2(1)],[campos(2),p2(2)],[campos(3),p2(3)],'b');
    if i>1
      plot3([pp(1),p2(1)],[pp(2),p2(2)],[pp(3),p2(3)],'b');
    end
    pp=p2;
  end
end
box on;
axis vis3d;
axis equal;

function tpts=tw(world,pts)
tpts=world.R*pts;
for i=1:size(world.T)
  tpts(i,:)=tpts(i,:)+world.T(i);
end


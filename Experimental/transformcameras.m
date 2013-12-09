% Convert coordinates of cdata to world coords
% See also http://www.vision.caltech.edu/bouguetj/calib_doc/htmls/parameters.html
% Given a point P=XXg=[Xg,Yg,Zg] in grid reference frame.
% The coordinate of the point in the camera reference frame, XXc=[Xc,Yc,Zc] = Rgc*XXg+Tgc,  
%                                                            XXg=Rgc^(-1)*(XXc-Tgc);
% The coordinate of the point in the world reference frame,  XXw=[Xw,Yw,Zw] = Rgw*XXg+Tgw,
%                                                         so XXw=Rgw*(Rgc^(-1)*(XXc-Tgc))+Tgw, 
%                                                         or XXw=(Rgw*Rgc^(-1))XXc + (Tgw-Rgw*Rgc^(-1)*Tgc)
%  Define Rcw=Rgw*Rgc^(-1),  Tcw=Tgw-Rcw*Tgc,  so XXw=Rcw*XXc+Tcw
function [p,world]=transformcameras(p)
% Find a transformation that moves all the grid-relative coordinates (in p.cam(i).extcal.gridposition) to be as close as possible to those in p.layout.cpos(:,1:2)
% Form a vector of the 3 rotations and 3 translations of the coordinate system
cpos=p.layout.cpos;
if size(cpos,1)~=6
  error('transformcameras only implemented for 2 cameras\n');
end
lastcamera=size(cpos,1);
midcamera=floor(lastcamera/2);

cpos(:,3)=p.layout.cposz;
for i=1:length(p.camera)
  gpos(i,1:3)=p.camera(i).extcal.gridposition;
end
% Augment with the points along the bottom of the grid at the same height as the cameras
gridbottom=p.calibration.target.dX*p.calibration.target.nX+p.calibration.target.bottomrowheight-mean(cpos(:,3));
gpos(end+1,:)=[gridbottom,0,0];
gpos(end+1,:)=[gridbottom,gridbottom,0];

world.R=eye(1);
world.T=zeros(3,1);
gw=transform(world,gpos');
  
% Find plane which intersects all the cameras and an equivalent height on the grid
for i=1:2
  A=gw';
  b=ones(size(A,1),1);
  x=A\b;
  fprintf('Equation of plane through all cameras and grid bottom:  %.3fx+%.3fy+%.3fz = 1, angle error=[%.1f %.1f] degrees\n', x, atand(x(1:2)/x(3)));
  rotaxis=cross(x,[0,0,1]);rotaxis=rotaxis/norm(rotaxis);
  rotangle=acos(dot(x/norm(x),[0,0,1]));
  fprintf('Rotation axis = [%.3f,%.3f,%.3f], angle=%.3f deg\n',rotaxis,rotangle*180/pi);
  if i==1
    fprintf('Rotating frame of reference to put all cameras, grid on xy plane\n');
    rot=rodrigues(rotaxis*rotangle);
    world.R=rot*world.R;
    world.T=rot*world.T;
    gw=transform(world,gpos');
  end
end

% Check for flatness
if std(gw(3,:))>0.010
  fprintf('**** Warning **** plane through cameras and grid bottom not flat -- standard deviation = %.0f mm\n', std(gw(3,:))*1000);
end

% Rotate world so that the cameras midcamera and midcamera+1 are along y-axis
dirmid=gw(:,midcamera+1)-gw(:,midcamera);dirmid(3)=0;dirmid=dirmid/norm(dirmid);
rotdir=cross(dirmid,[0,1,0]);rotdir=rotdir/norm(rotdir);
rotangle=acos(dot(dirmid,[0,1,0]));
fprintf('Rotating by [%.2f,%.2f,%.2f] degrees to align C3,C% along Y axis\n', rotangle.*rotdir*180/pi,midcamera,midcamera+1);
rot=rodrigues(rotangle*rotdir);
world.R=rot*world.R;    % Rgw
world.T=rot*world.T;
gw=transform(world,gpos');

% Make sure the x-axis is pointing the right way
if gw(end,1)<gw(1,1)
  fprintf('Flipping x-axis\n');
  rot=rodrigues([0,pi,0]);
  world.R=rot*world.R;    % Rgw
  world.T=rot*world.T;
  gw=transform(world,gpos');
end

% Translate world to put average C3,C4 at x=0, average of C1,C6 at y=0, average height at average cposz
offset=[-mean(gw(1,midcamera:midcamera+1)),-mean(gw(2,[1,lastcamera])),mean(cpos(1:lastcamera,3))-mean(gw(3,1:lastcamera))];
world.T=world.T+offset';
fprintf('Translate by [%.2f,%.2f,%.2f] to move cameras to correct position\n',offset);
gw=transform(world,gpos');

% Correct the layout
%roomwidth=gw(2,lastcamera)-gw(2,1)+0.2;  % 10cm beyond outer cameras
roomwidth=max(p.layout.active(:,2))-min(p.layout.active(:,2));
fprintf('Adjusting room size to width of %.2f m aligned with cameras 1 and %d at the edges\n', roomwidth, lastcamera);

% Map directions
forward=[0;0;1];   % In camera reference frame, direction it is facing
up=[0;-1;0];        % Up in camera reference frame
for i=1:size(cpos,1)
  ext=p.camera(i).extcal.extrinsic;
  Rcw=world.R*inv(ext.Rc);
  Tcw=world.T-Rcw*ext.Tc;
  cdir=Rcw*forward;
  mapup=Rcw*up;
  roll=atan(mapup(2)/mapup(3));   % Roll of the camera in radians
  clayout.cpos(i,:)=gw(1:2,i);
  clayout.cposz(i)=gw(3,i);
  clayout.cdir(i,:)=cdir(1:2);
  clayout.cdirz(i)=cdir(3);
  clayout.croll(i)=roll;
  p.camera(i).extcal.Rcw=Rcw;
  p.camera(i).extcal.Tcw=Tcw;
end

p.layout=layoutroom(roomwidth,max(p.layout.active(:,1)),'nled',size(p.layout.lpos,1),'clayout',clayout);

p.calibration.Rgw=world.R;
p.calibration.Tgw=world.T;
p.calibration.when=now;


% Transform from grid coordinates to world
function pw=transform(world,pg)
pw=world.R*pg+repmat(world.T,1,size(pg,2));
fprintf('Cameras at ');for cc=1:size(pw,2);fprintf('[%.3f,%.3f,%.3f] ',pw(:,cc));end;fprintf('\n');


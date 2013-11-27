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
cpos(:,3)=p.layout.cposz;
for i=1:length(p.camera)
  gpos(i,1:3)=p.camera(i).extcal.gridposition;
end

% Find tranform that minimizes distance between R*gpos+T and cpos
xinit=[0,0,0,0,0,0];
options=optimset('display','final','MaxFunEvals',5000,'MaxIter',5000,'TolX',1e-10,'TolFun',1e-10);
xfinal=fminsearch(@(z) calcdist(z,gpos,cpos)+targetheight(z,p.calibration.target.floor), xinit,options);
e1=calcdist(xfinal,gpos,cpos);
e2=targetheight(xfinal,p.calibration.target.floor);
fprintf('After optimization, camera position error = %.2g m, target height error=%.2g m\n', sqrt(e1), sqrt(e2));
world.T=xfinal(1:3)';   % Note that it should a column vector 
world.R=rodrigues(xfinal(4:6));
[world.esqd,world.mappos]=calcdist(xfinal,gpos,cpos);
for i=1:size(cpos,1)
  world.err(i)=norm(cpos(i,:)-world.mappos(i,:));
end

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
  fprintf('Camera %d position adjusted by %.1f m\n', i,norm([p.layout.cpos(i,:),p.layout.cposz(i)]-world.mappos(i,:)));
  p.layout.cpos(i,:)=world.mappos(i,1:2);
  p.layout.cposz(i)=world.mappos(i,3);
  fprintf('Camera %d direction vector adjusted by %.1f degrees\n', i,acosd(dot([p.layout.cdir(i,:),p.layout.cdirz(i)],cdir)));
  p.layout.cdir(i,:)=cdir(1:2);
  p.layout.cdirz(i)=cdir(3);
  fprintf('Camera %d roll adjusted by %.1f degrees\n', i,(roll-p.layout.croll(i))*180/pi);
  p.layout.croll(i)=roll;
  p.camera(i).extcal.Rcw=Rcw;
  p.camera(i).extcal.Tcw=Tcw;
end

p.calibration.Rgw=world.R;
p.calibration.Tgw=world.T;
p.calibration.when=now;



function [e,v]=calcdist(x,gpos,cpos)
T=x(1:3);
R=rodrigues(x(4:6));
v=(R*gpos')';   % The rotation matrix multiplies a column vector giving a column result, but we want to do it by rows with row results
for i=1:size(v,1)
  v(i,:)=v(i,:)+T;
end
e=sum((v(:)-cpos(:)).^2);

% Compute an error in the height of the part of the target that should be on the floor
function e=targetheight(x,floorpos)
T=x(1:3);
R=rodrigues(x(4:6));
v=(R*floorpos')'+T;
e=v(3)^2;
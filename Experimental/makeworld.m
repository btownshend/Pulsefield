% Convert coordinates of cdata to world coords
% See also http://www.vision.caltech.edu/bouguetj/calib_doc/htmls/parameters.html
% Given a point P=XXg=[Xg,Yg,Zg] in grid reference frame.
% The coordinate of the point in the camera reference frame, XXc=[Xc,Yc,Zc] = Rgc*XXg+Tgc,  
%                                                            XXg=Rgc^(-1)*(XXc-Tgc);
% The coordinate of the point in the world reference frame,  XXw=[Xw,Yw,Zw] = Rgw*XXg+Tgw,
%                                                         so XXw=Rgw*(Rgc^(-1)*(XXc-Tgc))+Tgw, 
%                                                         or XXw=(Rgw*Rgc^(-1))XXc + (Tgw-Rgw*Rgc^(-1)*Tgc)
%  Define Rcw=Rgw*Rgc^(-1),  Tcw=Tgw-Rcw*Tgc,  so XXw=Rcw*XXc+Tcw
% Make the world coordinates the same as camera 1's coordinate system
% XXc=XXw, so Rgw=Rgc and Tgw=Tgc
function world=makeworld(cdata)
world.R=cdata(1).r.Rc;
world.T=cdata(1).r.Tc;   % Tgw
% Now rotate 90 deg around x axis to make Y the direction the camera is pointing, Z is up and X is to right of camera
% So camera 2 should be at ~[-0.69,0,0]
rot=rodrigues([-pi/2,0,0]);
world.R=rot*world.R;    % Rgw
world.T=rot*world.T;
% Rotate world so cameras 1 and 2 are at the same height
cpos2=world.R*cdata(2).position+world.T;
angle=atan(cpos2(3)/cpos2(1));
fprintf('Rotating around Y axis by %.2f degrees to align cameras 1 and 2 to same height\n', angle*180/pi);
rot=rodrigues([0,angle,0]);
world.R=rot*world.R;    % Rgw
world.T=rot*world.T;
% Now make the line between cameras 1 and 2 aligned with the x axis
cpos2=world.R*cdata(2).position+world.T;
angle=atan(cpos2(2)/cpos2(1));
fprintf('Rotating around Z axis by %.2f degrees to align cameras 1 and 2 along x axis\n', angle*180/pi);
rot=rodrigues([0,0,-angle]);
world.R=rot*world.R;    % Rgw
world.T=rot*world.T;
% Rotate around x axis to put grid origin at correct height
gridz=.0625;
camz=0.047;
gpos=world.R*[cdata(1).target.dY*cdata(1).target.nX;0;0]+world.T;
angle=-atan((gpos(3)-(gridz-camz))/gpos(2));
fprintf('Rotating around X axis by %.2f degrees to place bottom grid at a height of %.2f\n', angle*180/pi,gridz);
rot=rodrigues([angle,0,0]);
world.R=rot*world.R;    % Rgw
world.T=rot*world.T;
% Move cameras 1 and 2 up to the known height above floor
world.T=world.T+[0,0,camz]';

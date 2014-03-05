% Convert a pixel coordinate on a camera's image to a direction vector
% Usage: dir=pix2dir(cam,pos)
%    cam - camera structure containing a distortion field
%    pos - pos(:,1) - horizontal position on sensor (0..sensorwdith-1)
%          pos(:,2) - veritcal position on sensor (0..sensorwdith-1)
%    dir(:,3) - unit vector direction in cameras frame of reference (right-hand convention)
%	   x is to right of camera
%          y is in direction camera is pointing
%          z is up
function dir=pix2dir(cam,pos)
if ~isfield(cam,'distortion')
  error('pix2dir: Camera structure does not contain a distortion model');
end
if ~isfield(cam.distortion,'mdl')
  fprintf('pix2dir: No distortion model in struct, assuming amcc-fisheye\n');
elseif ~strcmp(cam.distortion.mdl,'amcc-fisheye')
  error('pix2dir: Unknown distortion model: %s',cam.distortion.mdl);
end
xd=(pos-cam.distortion.cc)./cam.distortion.fc;
x=comp_fisheye_distortion(xd,cam.distortion.kc)';
dir(:,1)=x(:,1);
dir(:,2)=1;
dir(:,3)=x(:,2);
for i=1:size(dir,1)
  dir(i,:)=dir(i,:)/norm(dir(i,:));
end


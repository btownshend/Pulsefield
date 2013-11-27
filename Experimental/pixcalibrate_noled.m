% Calibrate positions, directions of cameras, setup pixcalib structure
% Usage: p=pixcalibrate_noled(p)
%
% Method:
% - Setup distortion parameters of each camera (from prior calibration)
%	- load from c%d-Results_left.mat using p.camera(:).physid for indexing
% - Acquire images of checkerboard from each camera and compute extrinsics
%	- needs manual intervention to click corners
%	- keep record of calibration in p.calibrate
% - Transform coordinates to global space where
%	- all cameras are at same height
%	- positions are set so they are all as close as possible to positions preset in layout
%	  (warn if any camera is more than a certain distance from planned position)
% - Update camera cpos, cdir, cposz, cdirz, croll in this space to calibrated values
% - Choose "virtual" LEDs (lpos) around perimeter of room except along x axis
% - Compute pixcalib using distortion model of each camera

function p=pixcalibrate_noled(p,varargin)
defaults=struct('newimages',false,'doplot',false,'maxcameramovement',0.5);
args=processargs(defaults,varargin);

target=struct('dX',0.03,'dY',0.03,'nX',16,'nY',16,'floor',[16*.03+.0625,0,0]);   % floor is a point in grid coordinate space that is on the floor (bottom edge)
for c=1:length(p.camera)
  cam=p.camera(c);
  if args.newimages || ~isfield(cam,'extcal') || isempty(cam.extcal)
    cam.distortion=load(sprintf('c%d-Results_left',cam.physid));
    I=arecont(cam.id);
    setfig('interactive');
    imshow(I.im);
    title('Click on top left, bottom right corners');
    [cx,cy]=ginput(2);
    I.bounds=round([cx,cy]);
    plot(I.bounds(:,1),I.bounds(:,2),'x');
    extrinsic=extrinsic_auto(I,target,cam.distortion,0,0);
    position=cam2grid(extrinsic,[0;0;0]);
    fprintf('Camera %d position is [%.2f,%.2f,%.2f]\n', cam.id, position);
    p.camera(c).distortion=cam.distortion;
    p.camera(c).extcal=struct('image',I,'extrinsic',extrinsic,'gridposition',position);
  else
    fprintf('Using existing extcal for camera %d\n',cam.id);
  end
end

if args.doplot
  setfig('Grid space');clf;
  hold on;
  plotworld(p);
end

% Construct the mapping from grid coordinates to world coordinates
p.calibration.target=target;
p=transformcameras(p);

if args.doplot
  setfig('World space');clf;
  hold on;
  plotworld(p,world);
  axis([-5,5,0,10,0,2]);
end


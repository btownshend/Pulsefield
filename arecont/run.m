addpath ~/Documents/Photography/Calibration/Camera/amcctoolbox/calibration
addpath ~/Documents/Photography/Calibration/Camera/amcctoolbox/amcctoolbox
addpath ~/Documents/Photography/Calibration/Camera/amcctoolbox/amcctoolbox/CornerFinder
addpath ~/Documents/Photography/Calibration/Camera/amcctoolbox

diary calibrate.log
%cams=[ 1 2 3 4 5 6 ];
cams=5;
wd=cd;
for i=1:length(cams)
  cd(wd);
  cam=cams(i);
  %%%%%%%%%%%%%%%%%%%% Calibrate Monocular Camera %%%%%%%%%%%%%%%%%%%%%%%%
  % Default image name
  calib_name = sprintf('c%d-',cam);

  % The name to save the results under
  save_name = [calib_name,'Results_left'];

  % Where the images are (forward slashes only, and must include a trailing slash)
  input_dir = sprintf('C%d/',cam);

  % Where the data will be saved (forward slashes only, and must include a trailing slash). This folder should already exist.
  output_dir = input_dir;

  if exist([output_dir,save_name,'.m'],'file')
    fprintf('%s already exists, skipping %s\n', [output_dir,save_name,'.m'],output_dir);
    continue;
  end
  
  % Image format: jpeg, bmp, tiff, png etc.
  format_image = 'jpg';

  % Length of each square of the checkerboard in the X direction (mm)
  dX = 30.0;
  % Length of each square of the checkerboard in the Y direction (mm)
  dY = 30.0;
  % X,Y direction is according to image coordinates. So X is in vertical direction and Y is in horizontal direction.

  % number of square corners of the checkerboard in the X direction
  nx_crnrs = 17;
  % number of square corners of the checkerboard in the Y direction
  ny_crnrs = 17;

  % tolerance in pixels of reprojection of checkerboard corners
  proj_tol = 10.0;

  % indicate whether or not to rotate the image by 180 degrees (0 no, 1 yes)
  rot_flag = 0;

  % indicate whether or not to use the fisheye calibration routine.
  fisheye = true;

  % indicate whether or not to use the third radial distortion term when doing a projective calibration
  k3_enable = false;

  % Calibrate the monocular camera automatically
  % and reject those images with error greater than proj_tol
  auto_mono_calibrator_efficient; % version 1.3a and before
                                  %auto_mono_calibrator_efficient(calib_name, save_name, input_dir, output_dir, format_image, dX, dY, nx_crnrs, ny_crnrs, proj_tol, rot_flag, fisheye, k3_enable); % version 1.3b and after

  % If desired, show the final reprojection error to the user by uncommenting the following line
end
diary off

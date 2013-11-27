
if ~exist('fc_left')|~exist('cc_left')|~exist('kc_left')|~exist('alpha_c_left'),
   fprintf(1,'No intrinsic camera parameters available for left camera.\n');
   return;
end;

if ~exist('fc_right')|~exist('cc_right')|~exist('kc_right')|~exist('alpha_c_right'),
   fprintf(1,'No intrinsic camera parameters available for right camera.\n');
   return;
end;

if ~exist('fisheye','var'),
    fisheye = false;
end

if (~exist('rotcam','var'))
    rotcam = zeros(size(camera_vec));
end

undistorted_dir = [input_dir 'undistorted/'];

if (~exist(undistorted_dir,'dir'))
    display(['Making an undistorted image directory at ''' undistorted_dir  '''...']);
    mkdir(undistorted_dir) % Make the directory if it doesn't already exist
end

kk = input('Which pair number would you like to examine (A single number only please)? ');

% Left image:

I = load_image(kk,calib_name_left,format_image_left,type_numbering_left,image_numbers_left,N_slots_left, input_dir, 0);

fprintf(1,'Computing the undistorted left image...')

[I1] = rect(I,eye(3,3),fc_left,cc_left,kc_left,alpha_c_left,KK_left,fisheye);

% rotate if the flag is true
if rotcam(1)
    display('Rotating image 180 degrees...')
    I1 = imrotate(I1,180);
end

fprintf(1,'done\n');

left_name = write_image(I1,kk,[calib_name_left '_undistorted'],format_image_left,type_numbering_left,image_numbers_left,N_slots_left, undistorted_dir);

fprintf(1,'\n');

% Right image:

I = load_image(kk,calib_name_right,format_image_right,type_numbering_right,image_numbers_right,N_slots_right, input_dir,0);

fprintf(1,'Computing the undistorted right image...\n');

[I2] = rect(I,eye(3,3),fc_right,cc_right,kc_right,alpha_c_right,KK_right,fisheye);

% rotate if the flag is true
if rotcam(2)
    display('Rotating image 180 degrees...')
    I2 = imrotate(I2,180);
end

right_name = write_image(I2,kk,[calib_name_right '_undistorted'],format_image_right,type_numbering_right,image_numbers_right,N_slots_right, undistorted_dir);

fprintf(1,'\n');



%% Get the homography matrix
H_from_calibration;

% Left Camera
M0 = [1 0 0 0;
      0 1 0 0;
      0 0 1 0];

K0 = [fc_left(1) alpha_c_left cc_left(1);
      0  fc_left(2)   cc_left(2);
      0  0    1];

P0 = K0*M0;

% Right Camera
M1 = H(1:3,1:4);

%M1 = [H(1:3,1:3) H(1:3,1:3)*H(1:3,4)];


K1 = [fc_right(1) alpha_c_right cc_right(1);
      0  fc_right(2)   cc_right(2);
      0  0    1];

P1 = K1*M1;
pmr = P1;


im0 = imread(left_name);
im1 = imread(right_name);

% Compute F from P's
F = vgg_F_from_P(P0, P1);

% Display
vgg_gui_F(im0, im1, F');
 disp('Computed epipolar geometry. Move the mouse to verify')

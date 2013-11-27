% First check whether all the relevent variables exist
if ~exist('fc_left')|~exist('cc_left')|~exist('kc_left')|~exist('alpha_c_left'),
   fprintf(1,'No intrinsic camera parameters available for left camera.\n');
   return;
end;

if ~exist('fc_right')|~exist('cc_right')|~exist('kc_right')|~exist('alpha_c_right'),
   fprintf(1,'No intrinsic camera parameters available for right camera.\n');
   return;
end;

if(~exist('cv_path','var'))
    cv_path = [output_dir 'opencv/'];
end

if(~exist('ros_path','var'))
    ros_path = [output_dir 'ros/'];
end

if(~exist('cam0_names','var'))
    cam0_names = 'cam0_';
end

if(~exist('cam1_names','var'))
    cam1_names = 'cam1_';
end


% Export the intrinsics matrix and distortions to XML files
opencv_intrinsics_from_calibration(cam0_names, kc_left, fc_left, cc_left, cv_path);
opencv_intrinsics_from_calibration(cam1_names, kc_right, fc_right, cc_right, cv_path);


% Export the rectification matrices to XML files
opencv_undistortionmap_from_calibration(cam0_names, kc_left, fc_left, cc_left, alpha_c_left, nx, ny, cv_path);
opencv_undistortionmap_from_calibration(cam1_names, kc_right, fc_right, cc_right, alpha_c_right, nx, ny, cv_path);


% Generate an unrectified set of yaml files
rect = eye(3,3);

ros_yaml_from_calibration(cam0_names, kc_left, fc_left, cc_left, alpha_c_left, nx, ny, rect, om, T, ros_path);
ros_yaml_from_calibration(cam1_names, kc_right, fc_right, cc_right, alpha_c_right, nx, ny, rect, om, T, ros_path);

% Now generate a rectified set of yaml files

% Differentiate the camera names
save_name_rect1 = [cam0_names '_rect'];
save_name_rect2 = [cam1_names '_rect'];

% generate the rectified calibration matrices
generate_rectification_matrices;

% export
ros_yaml_from_calibration(save_name_rect1, kc_left_rect, fc_left_rect, cc_left_rect, alpha_c_left_rect, nx, ny, R_L, zeros(3,1), zeros(3,1), ros_path);
ros_yaml_from_calibration(save_name_rect2, kc_right_rect, fc_right_rect, cc_right_rect, alpha_c_right_rect, nx, ny, R_R, om_rect, T_rect, ros_path);
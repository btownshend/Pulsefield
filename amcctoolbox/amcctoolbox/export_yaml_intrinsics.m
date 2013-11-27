
if(~exist('save_name','var'))
    save_name = 'camera';
end

if ~exist('fc')|~exist('cc')|~exist('kc')|~exist('cc')|~exist('alpha_c'),
   fprintf(1,['No intrinsic camera parameters available for camera ' save_name '\n']);
   return;
end;

if (~exist('ros_path','var'))
    % Get the output name from the user
    ros_path = [output_dir 'ros/'];
end

%Since this a monocular calibration, set all the stereo parameters as
%identity

% rectification matrix (identity for monocular camera)
rect = eye(3,3);

% rotation and translation
om = [0 0 0];
T = [0 0 0]';

% Export the intrinsics matrix and distortions to XML files
ros_yaml_from_calibration(save_name, kc, fc, cc, alpha_c, nx, ny, rect, om, T, ros_path);
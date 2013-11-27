
if ~exist('fc')|~exist('cc')|~exist('kc')|~exist('cc')|~exist('alpha_c'),
   fprintf(1,['No intrinsic camera parameters available for camera ' save_name '\n']);
   return;
end;

cv_path = [output_dir 'opencv/'];

if(~exist('save_name','var') 
    save_name = 'camera';
end

% Export the intrinsics matrix and distortions to XML files
opencv_intrinsics_from_calibration(save_name, kc, fc, cc, cv_path);

% Export the rectification matrices to XML files
opencv_undistortionmap_from_calibration(save_name, kc, fc, cc, alpha_c, nx, ny, cv_path)
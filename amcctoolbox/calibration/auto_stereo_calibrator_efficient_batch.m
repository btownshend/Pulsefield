% AUTO_STEREO_CALIBRATOR_EFFICIENT_BATCH(camera_vec, input_dir, output_dir, format_image, dX, dY, proj_tol, rotcam, cam0_names, cam1_names, fisheye, k3_enable)
% Performs an automatic stereo calibration by performing two monocular
% calibrations in seperate worker threads for efficiency, then performs a
% stereo calibration on the data
% Dependant on the RADOCCToolbox
% Dependent on having the parallel computing toolbox
% camera_vec -> Vector. The numerical index of the cameras. For stereo, usually [0 1]
% input-dir -> String. The directory where the images are stored
% output-dir -> String. The directory where the matlab output will be saved
% format_image -> String. The format of the images: 'bmp', 'jpg' etc.
% dX -> int. The length of each checkerboard square in mm in the X direction
% dY -> int. The length of each checkerboard square in mm in the Y direction
% nx_crnrs -> int. Number of expected corners in the X direction
% ny_crnrs -> int. Number of expected corners in the Y direction
% proj_tol -> float. The tolerance for reprojection of corners
% rotcam -> logical (1 or 0). Forces image to be rotated by 180 degrees. Useful for UAV cameras.
% cam0_names -> String. The base name of the image files for the base camera
% cam1_names -> String. The base name of the image files for the base camera
% fisheye -> logical (1 or 0). Performs a fisheye calibration instead of a standard calibration
% k3_enable -> logical (1 or 0). Enables the k3 term for highly distorted projective lenses

% Copyright (c) Michael Warren 2013

% This file is part of the AMMCC Toolbox.

% The AMMCC Toolbox is free software: you can redistribute it and/or modify
% it under the terms of the GNU Lesser General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
% 
% The AMMCC Toolbox is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Lesser General Public License for more details.
% 
% You should have received a copy of the GNU Lesser General Public License
% along with the AMMCC Toolbox.  If not, see <http://www.gnu.org/licenses/>.
    


function auto_stereo_calibrator_efficient_batch(camera_vec, input_dir, output_dir, format_image, dX, dY, nx_crnrs, ny_crnrs, proj_tol, rotcam, cam0_names, cam1_names, fisheye, k3_enable)

% Explicitly clear any existing job variables
clear jobi jobj;

% Make sure the vector is only of size 2
cam_vec_sz = size(camera_vec);
rotcam_sz = size(rotcam);

% Force the fisheye variable to false if it doesn't exist
if (~exist('fisheye','var'))
    fisheye = false;
end

% Force the k3_enable variable to false if it doesn't exist
if (~exist('k3_enable','var'))
    k3_enable = false;
end


% Argument sanity checking
if (cam_vec_sz(2) ~= rotcam_sz(2))
    display('Error: Your camera vector and rotation vector are of different sizes. Cannot continue.');
    return;
end
if (cam_vec_sz(2) > 2 || rotcam_sz(2) > 2)
    display('Error: You have specified more than 2 cameras to be stereo calibrated. Did you want to use auto_multi_calibrator_efficient?');
    return;
elseif (cam_vec_sz(2) < 2 || rotcam_sz(2) < 2)
    display('Error: You have specified less than 2 cameras to be stereo calibrated. Did you want to use auto_mono_calibrator_efficient?');
    return;
end

% Default image names
if (~exist('cam0_names','var'))
    cam0_names = ['cam' num2str(camera_vec(1)) '_image'];
end
if (~exist('cam1_names','var'))
    cam1_names = ['cam' num2str(camera_vec(2)) '_image'];
end

% Make sure the output directory exists
if (~exist([output_dir],'dir'))
    display(['Making a calibration directory at ''' output_dir '...']);
    mkdir(output_dir) % Make the directory if it doesn't already exist
end

checkerboard_set = 0;

%%%%%%%%%%%%%%%%%% Monocular Calibrations %%%%%%%%%%%%%%%%%%%%%%
c = parcluster();


%%%%%%%%%%%%%%%%%%%% Calibrate Camera 0 %%%%%%%%%%%%%%%%%%%%%%%%

% Set the extra k3 term if necessary
if (k3_enable && ~fisheye)
    est_dist = [1 1 1 1 1]';
else
    est_dist = [1 1 1 1 0]';
end

% Camera 0 specific parameters
calib_name = cam0_names;
save_name = [output_dir 'Calib_Results_' num2str(camera_vec(1))];

% Calibrate the monocular camera automatically
% and reject those images with error greater than proj_tol
rot_flag = rotcam(1);

display(['Starting calibration of camera ' num2str(camera_vec(1)) ' as a batch job...']);

% perform the calibration
jobi = batch('mono_batch_launcher');


%%%%%%%%%%%%%%%%%%%% Calibrate Camera 1 %%%%%%%%%%%%%%%%%%%%%%%%

% Set the extra k3 term if necessary
if (k3_enable && ~fisheye)
    est_dist = [1 1 1 1 1]';
else
    est_dist = [1 1 1 1 0]';
end

% Camera 1 specific parameters
calib_name = cam1_names;
save_name = [output_dir 'Calib_Results_' num2str(camera_vec(2))];

% Calibrate the monocular camera automatically
% and reject those images with error greater than proj_tol
rot_flag = rotcam(2);

display(['Starting calibration of camera ' num2str(camera_vec(2)) ' as a batch job...']);

% perform the calibration
jobj = batch('mono_batch_launcher');

%%%%%%%%%%%%%%%%%%%% Wait for the jobs to finish and reload the workspaces %%%%%%%%%%%%%%%%%%%%%%%%

% Wait for camera 0
display(['Waiting for monocular calibration of camera ' num2str(camera_vec(1)) ' to finish (this may take some time)...']);
wait(jobi);   % Wait for the job to finish
display('...done');
diary(jobi);   % Display the diary
load(jobi); % Reload the workspace

% generate the suppression list
eval(['cam' num2str(camera_vec(1)) '_suppress_list = active_images;']);
save([output_dir 'cam' num2str(camera_vec(1)) '_suppress_list'], ['cam' num2str(camera_vec(1)) '_suppress_list']);

% Wait for camera 1 (should happen almost immediately)
display(['Waiting for monocular calibration of camera ' num2str(camera_vec(1)) ' to finish...']);
wait(jobj);   % Wait for the job to finish
display('...done');
diary(jobj);   % Display the diary
load(jobj); % Reload the workspace

% generate the suppression list
eval(['cam' num2str(camera_vec(2)) '_suppress_list = active_images;']);
save([output_dir 'cam' num2str(camera_vec(2)) '_suppress_list'], ['cam' num2str(camera_vec(2)) '_suppress_list']);

% delete the workers
delete(jobi);
delete(jobj);

%%%%%%%% Normalise the variable naming for the next section %%%%%%%%%

% reload
% load([output_dir 'cam' num2str(camera_vec(1)) '_suppress_list']);
% load([output_dir 'cam' num2str(camera_vec(2)) '_suppress_list']);

eval(['suppress_list0 = cam' num2str(camera_vec(1)) '_suppress_list;']);
eval(['suppress_list1 = cam' num2str(camera_vec(2)) '_suppress_list;']);

%%%%%%%% Make sure that matching images are suppressed %%%%%%%%%
fprintf('\n********************** Generating matching suppression lists ********************\n');
len_list0 = size(suppress_list0);
len_list1 = size(suppress_list1);

if (len_list0(2) ~= len_list1(2))
    display('Error: The two images lists are not equal. Check to make sure all images have been found.');
    return;
end

for ii=1:1:len_list0(2)
    if (suppress_list0(ii) ~= suppress_list1(ii))
        suppress_list0(ii) = 0;
        suppress_list1(ii) = 0;
    end
end

% Save the changes
% save(['cam' num2str(camera_vec(1)) '_suppress_list'], ['suppress_list0']);
% save(['cam' num2str(camera_vec(2)) '_suppress_list'], ['suppress_list1']);


%%%%%%%%%%%%%%%%%%%%% Stereo Calibration %%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%% Reload and recalibrate each camera based on the suppression list %%%%
fprintf('\n********************** Recalibration based on suppression list; preparing for stereo calibration... ********************\n');

% Left camera

% Set the extra k3 term if necessary
if (k3_enable && ~fisheye)
    est_dist_left = [1 1 1 1 1]';
else
    est_dist_left = [1 1 1 1 0]';
end

load([output_dir 'Calib_Results_' num2str(camera_vec(1)) '.mat']);
eval(['active_images = suppress_list0;']);

% Perform the optimisation
if fisheye
    fprintf('\n\n ***** Runing Calibration for fisheye lens *****\n\n')
    go_calib_optim_fisheye_no_read;
else
    fprintf('\n\n ***** Runing Calibration for normal lens *****\n\n')
    go_calib_optim;
end

% Save the output
save_name = [output_dir 'Calib_Results_left'];
saving_calib_auto;

% Right camera

% Set the extra k3 term if necessary
if (k3_enable && ~fisheye)
    est_dist_right = [1 1 1 1 1]';
else
    est_dist_right = [1 1 1 1 0]';
end

load([output_dir 'Calib_Results_' num2str(camera_vec(2)) '.mat']);
eval(['active_images = suppress_list1 ;']);

% Perform the optimisation
if fisheye
    fprintf('\n\n ***** Runing Calibration for fisheye lens *****\n\n')
    go_calib_optim_fisheye_no_read;
else
    fprintf('\n\n ***** Runing Calibration for normal lens *****\n\n')
    go_calib_optim;
end

% Save the output
save_name = [output_dir 'Calib_Results_right'];
saving_calib_auto;

%%%% Do the stereo calibration %%%%
fprintf('\n********************** Performing Stereo Calibration ********************\n');
calib_file_name_left = [output_dir 'Calib_Results_left.mat'];
calib_file_name_right = [output_dir 'Calib_Results_right.mat'];

fprintf('Loading the monocular results files');
%load_stereo_calib_files2_auto; % Load the monocular files
load_stereo_calib_files_auto; % Load the monocular files


% Don't recompute the intrinsics. This is necessary for future stereo
% calibrations in multicamera setups. May have to change.
recompute_intrinsic_left = 0;
recompute_intrinsic_right = 0;

%calib_stereo_auto2; % Perform stereo calibration
%fprintf('Showing the stereo calibration before optimisation...');
%show_stereo_calib_results; % Show the values before calibration

go_calib_stereo; % Perform stereo calibration
%calib_stereo_auto2; 

% Manually change back the rotation if we rotated one camera
if ((rotcam(1) == 1 && rotcam(2) == 0) || (rotcam(2) == 1 && rotcam(1) == 0))
    om(3) = om(3) + pi;
end

%fprintf('Showing the stereo calibration after optimisation...');
%show_stereo_calib_results; % Show the values after calibration

fprintf('Saving the stereo calibration results...');
saving_stereo_calib; % Save the stereo calibration


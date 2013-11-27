% auto_stereo_calibration - takesa set of images from a camera pair in the
% standard multicamera format, detects checkerboards, calibrates
% individually, then calibrates in stereo to produce a final output
% Dependant on the RADOCCToolbox

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
    

function auto_stereo_calibrator(input_dir, output_dir, format_image, dX, dY, proj_tol)

% Default image names
cam0_names = 'cam0_image';
cam1_names = 'cam1_image';

% Change to the deafult output directory
cd(output_dir);

%%%%%%%%%%%%%%%%%%%%%%%%% Monocular Calibrations %%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%% Calibrate Camera 0 %%%%%
calib_name = cam0_names;

%load the images
data_calib_auto;

% Automatically detect corners
click_calib_auto;

% Perform the optimisation
go_calib_optim;

% Recursively remove cameras with pixel values outside the required range
valid = false;
max_err = 0.0;
max_err_img = -1;
while (valid == false)
    extract_error
    max_err_img
    max_err
    if (max_err_img == -1)
        valid = true;
    else
        % Suppress the image with the biggest error
        ima_numbers = max_err_img;
        suppress_auto;
        % Recalibrate
        go_calib_optim;
    end
end

%analyse_error;

cam0_suppress_list = active_images;

% Save the output
save_name = 'Calib_Results_left';
saving_calib_auto;

%%%%%%%%%%%%%%%%%%%% Calibrate Camera 1 %%%%%
calib_name = cam1_names;

%load the images
data_calib_auto;

% Automatically detect corners
click_calib_auto;

% Perform the optimisation
go_calib_optim;

% Recursively remove cameras with pixel values outside the required range
valid = false;
max_err = 0.0;
max_err_img = -1;
while (valid == false)
    extract_error
    max_err_img
    max_err
    if (max_err_img == -1)
        valid = true;
    else
        % Suppress the image with the biggest error
        ima_numbers = max_err_img;
        suppress_auto;
        % Recalibrate
        go_calib_optim;
    end
end

%analyse_error;

cam1_suppress_list = active_images;

% Save the output
save_name = 'Calib_Results_right';
saving_calib_auto;

%%%%%%%%%%%%%%%%%% Make sure that matching images are suppressed %%%%%%%%%%%%%%%%%%
len_list0 = size(cam0_suppress_list);
len_list1 = size(cam1_suppress_list);

if (len_list0(2) ~= len_list1(2))
    display('Error: The two images lists are not equal. Check to make sure all images have been found.');
end

for ii=1:1:len_list0(2)
    if (cam0_suppress_list(ii) ~= cam1_suppress_list(ii))
        cam0_suppress_list(ii) = 0;
        cam1_suppress_list(ii) = 0;
    end
end

%%%%%%%%%%%% Recalibrate each camera based on the suppression list %%%%%%

% Left camera
load('Calib_Results_left.mat');
active_images = cam0_suppress_list;
% Perform the optimisation
go_calib_optim;
% Save the output
save_name = 'Calib_Results_left';
saving_calib_auto;

% Right camera
load('Calib_Results_right.mat');
active_images = cam1_suppress_list;
% Perform the optimisation
go_calib_optim;
% Save the output
save_name = 'Calib_Results_right';
saving_calib_auto;


%%%%%%%%%%%%%%%%%%%%% Stereo Calibration %%%%%%%%%%%%%%%%%%%%%%%%%%%
calib_file_name_left = 'Calib_Results_left.mat';
calib_file_name_right = 'Calib_Results_right.mat';

%load_stereo_calib_files2_auto; % Load the monocular files
load_stereo_calib_files_auto; % Load the monocular files

%calib_stereo_auto2; % Perform stereo calibration

%show_stereo_calib_results; % Show the values before calibration

go_calib_stereo
%calib_stereo_auto2; % Perform stereo calibration

saving_stereo_calib; % Save the stereo calibration



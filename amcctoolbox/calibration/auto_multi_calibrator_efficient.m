% AUTO_MULTI_CALIBRATOR_EFFICIENT(camera_vec, input_dir, output_dir, format_image, dX, dY, nx_crnrs, ny_crnrs, proj_tol, rotcam)
% Performs an automatic multi-camera calibration by performing multiple
% stereo calibrations
% Dependant on the RADOCCToolbox
% camera_vec -> Vector. The numerical index of the cameras. For stereo
% usually [0 1], but can be any size, like [0 1 2 3].
% input-dir -> String. The directory where the images are stored
% output-dir -> String. The directory where the matlab output will be saved
% format_image -> String. The format of the images: 'bmp', 'jpg' etc.
% dX -> int. The length of each checkerboard square in mm in the X direction
% dY -> int. The length of each checkerboard square in mm in the Y direction
% nx_crnrs -> int. Number of expected corners in the X direction
% ny_crnrs -> int. Number of expected corners in the Y direction
% proj_tol -> float. The tolerance for reprojection of corners
% rotcam -> logical (1 or 0). Forces image to be rotated by 180 degrees. Useful for UAV cameras.
% cam_names -> Vector of Strings. The base names of the image files for each camera
% fisheye -> logical (1 or 0). Performs a fisheye calibration instead of a standard calibration
% k3_enable -> logical (1 or 0). Enables the k3 term for highly distorted projective lenses
% batch -> logical. Whether to use the batch stereo calibrator or not. Depends on having the parallel computing toolbox

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
    


function auto_multi_calibrator_efficient(camera_vec, input_dir, output_dir, format_image, dX, dY, nx_crnrs, ny_crnrs, proj_tol, rotcam, cam_names, fisheye, k3_enable, batch)

% Force the fisheye variable to false if it doesn't exist
if (~exist('fisheye','var'))
    fisheye = false;
end

% Force the k3_enable variable to false if it doesn't exist
if (~exist('k3_enable','var'))
    k3_enable = false;
end

% Force the batch variable to false if it doesn't exist
if (~exist('batch','var'))
    batch = false;
end

% Force the batch variable to false if it doesn't exist
if (~exist('cam_names','var'))
    cam_names = '';
end

cam_vec_sz = size(camera_vec);
rotcam_sz = size(rotcam);
cam_names_sz = size(cam_names);

% Argument sanity checking
if (cam_vec_sz(1) ~= rotcam_sz(1) || cam_vec_sz(2) ~= rotcam_sz(2))
    display('Error: Your camera vector and rotation vector are of different sizes. Cannot continue.');
    return;
end

% Argument sanity checking
if (cam_names_sz(1) && size(unique(camera_vec),1) ~= cam_names_sz(1))
    display('Error: Your camera vector and naming vector are of different sizes. Cannot continue.');
    return;
end

for ii = 1:1:cam_vec_sz(2)
    temp_vec = [camera_vec(1,ii) camera_vec(2,ii)];
    rotcam_temp_vec = [rotcam(1,ii) rotcam(2,ii)];
    
    % Default image names
    if (~exist('cam_names','var') || ~cam_names_sz(1))
        cam0_names = ['cam' num2str(temp_vec(1)) '_image'];
        cam1_names = ['cam' num2str(temp_vec(2)) '_image'];
    else
        cam0_names = cam_names(temp_vec(1)+1,:);
        cam1_names = cam_names(temp_vec(2)+1,:);
    end
    
    display(['Stereo calibrating cameras ' num2str(camera_vec(1,ii)) ' and ' num2str(camera_vec(2,ii)) '...']);
    if(batch)
        auto_stereo_calibrator_efficient_batch(temp_vec, input_dir, output_dir, format_image, dX, dY, nx_crnrs, ny_crnrs, proj_tol, rotcam_temp_vec, cam0_names, cam1_names, fisheye, k3_enable);
    else
        auto_stereo_calibrator_efficient(temp_vec, input_dir, output_dir, format_image, dX, dY, nx_crnrs, ny_crnrs, proj_tol, rotcam_temp_vec, cam0_names, cam1_names, fisheye, k3_enable);
    end
end;
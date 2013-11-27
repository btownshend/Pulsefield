% An automatic Opencv Intrinsics and distortions file generator for use in 
% other applications

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

function opencv_intrinsics_from_calibration(save_name, kc, fc, cc, cv_path)

path_exists = 0;
if (~exist('cv_path'))
    % Get the output name from the user
    path_exists = 1;
    cv_path = input('Enter the output directory (or specify it with ''cv_path='') ','s');
end

% Make sure the output directory exists
if (~exist(cv_path,'dir'))
    display(['Making an intrinsics directory at ''' cv_path  '''...']);
    mkdir(cv_path) % Make the directory if it doesn't already exist
end

%%%%%%%%%% Camera Distortions %%%%%%%%%
cam_dist_fid = fopen([cv_path '/' save_name '_distortion.xml'], 'wt');
cam_dists = ['    ' num2str(kc(1))  ' ' num2str(kc(2)) ' '...
    num2str(kc(3)) ' ' num2str(kc(4)) ' ' num2str(kc(5))];
cam_dist_string = char('<?xml version="1.0"?>',...
'<opencv_storage>',...
'<Distortion type_id="opencv-matrix">',...
'  <rows>5</rows>',...
'  <cols>1</cols>',...
'  <dt>f</dt>',...
'  <data>',...
cam_dists,...
'  </data></Distortion>',...
'</opencv_storage>');

% Write it to the file
l = size(cam_dist_string);
for ii = 1:1:l(1)
    fprintf(cam_dist_fid,'%s\r\n',cam_dist_string(ii,:));
end

fclose(cam_dist_fid);

%%%%%%%%%% Camera Intrinsics %%%%%%%%%
cam_int_fid = fopen([cv_path '/' save_name '_intrinsics.xml'], 'wt');
cam_ints = ['    ' num2str(fc(1))  ' ' num2str(0.0) ' '...
    num2str(cc(1)) ' ' num2str(0.0) ' ' num2str(fc(2)) ' '...
    num2str(cc(2)) ' ' num2str(0.0) ' ' num2str(0.0) ' ' num2str(1.0)];
cam_int_string = char('<?xml version="1.0"?>',...
'<opencv_storage>',...
'<Intrinsics type_id="opencv-matrix">',...
'  <rows>3</rows>',...
'  <cols>3</cols>',...
'  <dt>f</dt>',...
'  <data>',...
cam_ints,...
'  </data></Intrinsics>',...
'</opencv_storage>');

% Write it to the file
l = size(cam_int_string);
for ii = 1:1:l(1)
    fprintf(cam_int_fid,'%s\r\n',cam_int_string(ii,:));
end

fclose(cam_int_fid);

if (path_exists)
    clear cv_path;
end
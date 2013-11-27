% An automatic ROS YAML intrinsics and distortions file generator for use in 
% other applications
function ros_yaml_from_calibration(cam_name, kc, fc, cc, alpha_c, nx, ny, rect, om, T, ros_path)

path_exists = 0;
if (~exist('ros_path','var'))
    % Get the output name from the user
    path_exists = 1;
    ros_path = input('Enter the output directory (or specify it with ''ros_path='') ','s');
end

% Make sure the output directory exists
if (~exist(ros_path,'dir'))
    display(['Making a ROS calibration directory at ''' ros_path  '''...']);
    mkdir(ros_path) % Make the directory if it doesn't already exist
end

K = [fc(1) alpha_c*kc(1) cc(1);
    0 fc(2) cc(2);
    0 0 1];


rod = rodrigues(om);

% Generate the projection matrix
P = K*[rod -rod*T];

%%%%%%%%%% Write Camera Distortions %%%%%%%%%
cam_int_fid = fopen([ros_path '/' cam_name '.yaml'], 'wt');

rec_string = ['    data: [' num2str(rect(1,1)) ', ' num2str(rect(1,2)) ', ' num2str(rect(1,3)) ', ' num2str(rect(2,1)) ', ' num2str(rect(2,2)) ', ' num2str(rect(2,3)) ', ' num2str(rect(3,1)) ', ' num2str(rect(3,2)) ', ' num2str(rect(3,3)) ']'];

P_string = ['    data: [' num2str(P(1,1)) ', ' num2str(P(1,2)) ', ' num2str(P(1,3)) ', ' num2str(P(1,4)) ', ' num2str(P(2,1)) ', ' num2str(P(2,2)) ', ' num2str(P(2,3)) ', ' num2str(P(2,4)) ', ' num2str(P(3,1)) ', ' num2str(P(3,2)) ', ' num2str(P(3,3)) ', ' num2str(P(3,4)) ']'];

cam_int_string = char(['image_width: ' num2str(nx)],...
['image_height: ' num2str(ny)],...
['camera_name: ' cam_name],...
'camera_matrix: ',...
'    rows: 3',...
'    cols: 3',...
['    data: [' num2str(fc(1)) ', ' num2str(alpha_c) ', ' num2str(cc(1)) ', 0, ' num2str(fc(2)) ', ' num2str(cc(2)) ' , 0, 0, 1]'],...
'distortion_model: plumb_bob',...
'distortion_coefficients:',...
'    rows: 1',...
'    cols: 5',...
['    data: [' num2str(kc(1)) ', ' num2str(kc(2)) ', ' num2str(kc(3)) ', ' num2str(kc(4)) ', ' num2str(kc(5)) ']'],...
'rectification_matrix:',...
'    rows: 3',...
'    cols: 3',...
rec_string,...
'projection_matrix:',...
'    rows: 3',...
'    cols: 4',...
P_string);

% Write it to the file
l = size(cam_int_string);
for ii = 1:1:l(1)
    fprintf(cam_int_fid,'%s\r\n',cam_int_string(ii,:));
end

fclose(cam_int_fid);

%%%%%%%%%% Done %%%%%%%%%

if (path_exists)
    clear ros_path;
end
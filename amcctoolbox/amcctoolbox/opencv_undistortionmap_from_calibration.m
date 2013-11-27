% An automatic Opencv undistortion map file generator for use by other
% applications
function opencv_undistortionmap_from_calibration(cam_name, kc, f, c, alpha, nr, nc, cv_path)

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


%%%%%%%%%% Undistortion map %%%%%%%%%%%


% Note: R is the motion of the points in space
% So: X2 = R*X where X: coord in the old reference frame, X2: coord in the new ref frame.


if ~exist('KK_new'),
   KK_new = [f(1) alpha*f(1) c(1);0 f(2) c(2);0 0 1];
end;



[mx,my] = meshgrid(1:nc, 1:nr);
px = reshape(mx',nc*nr,1);
py = reshape(my',nc*nr,1);

rays = inv(KK_new)*[(px - 1)';(py - 1)';ones(1,length(px))];


% Rotation: (or affine transformation):

rays2 = rays;

x = [rays2(1,:)./rays2(3,:);rays2(2,:)./rays2(3,:)];


% Add distortion:
 if ~exist('fisheye')
     fisheye = 0;
 end

if fisheye
    fprintf('\n\n Fisheye distortion model is used here! \n\n')
    xd = apply_fisheye_distortion(x,kc);
else
    fprintf('\n\n Simple distortion model is used here! \n\n')
    xd = apply_distortion(x,kc);
end



% Reconvert in pixels:

px2 = f(1)*(xd(1,:)+alpha*xd(2,:))+c(1);
py2 = f(2)*xd(2,:)+c(2);

% Reshape to an image
pxout = reshape(px2,nr,nc);
pyout = reshape(py2,nr,nc);


%%%%%%%%%% Undistortion map X %%%%%%%%%%%
cam_dist_fid = fopen([cv_path '/' cam_name '_undistortion_x.xml'], 'wt');

begin_string = char('<?xml version="1.0"?>',...
'<opencv_storage>',...
'<Undistortion type_id="opencv-matrix">',...
'  <rows>',...
num2str(nr),...
'</rows>',...
'  <cols>',...
num2str(nc),...
'</cols>',...
'  <dt>f</dt>',...
'  <data>');

end_string = char('  </data></Undistortion>',...
'</opencv_storage>');

% Write it to the file
l = size(begin_string);
for ii = 1:1:l(1)
    fprintf(cam_dist_fid,'%s\r\n',begin_string(ii,:));
end

pxoutstr = num2str(pxout');
l = size(pxoutstr);
for ii = 1:1:l(1)
    fprintf(cam_dist_fid,'%s\r\n',pxoutstr(ii,:));
end

l = size(end_string);
for ii = 1:1:l(1)
    fprintf(cam_dist_fid,'%s\r\n',end_string(ii,:));
end

fclose(cam_dist_fid);

%%%%%%%%%% End undistortion map x %%%%%%%%%

%%%%%%%%%% Undistortion map y %%%%%%%%%%%
cam_dist_fid = fopen([cv_path '/' cam_name '_undistortion_y.xml'], 'wt');

begin_string = char('<?xml version="1.0"?>',...
'<opencv_storage>',...
'<Undistortion type_id="opencv-matrix">',...
'  <rows>',...
num2str(nr),...
'</rows>',...
'  <cols>',...
num2str(nc),...
'</cols>',...
'  <dt>f</dt>',...
'  <data>');

end_string = char('  </data></Undistortion>',...
'</opencv_storage>');

% Write it to the file
l = size(begin_string);
for ii = 1:1:l(1)
    fprintf(cam_dist_fid,'%s\r\n',begin_string(ii,:));
end

pyoutstr = num2str(pyout');
l = size(pyoutstr);
for ii = 1:1:l(1)
    fprintf(cam_dist_fid,'%s\r\n',pyoutstr(ii,:));
end

l = size(end_string);
for ii = 1:1:l(1)
    fprintf(cam_dist_fid,'%s\r\n',end_string(ii,:));
end

fclose(cam_dist_fid);

%%%%%%%%%% End undistortion map y %%%%%%%%%


if (path_exists)
    clear cv_path;
end;
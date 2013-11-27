if ~exist('fc_right')|~exist('cc_right')|~exist('kc_right')|~exist('alpha_c_right')|~exist('fc_left')|~exist('cc_left')|~exist('kc_left')|~exist('alpha_c_left')|~exist('om')|~exist('T')
    
    if exist('Calib_Results_stereo.mat')~=2,
        fprintf(1,'No stereo calibration data.\n');
        return;
    else
        fprintf(1,'\nLoading the stereo calibration file Calib_Results_stereo.mat.\n');
        load Calib_Results_stereo; % Load the stereo calibration result
    end;
    
end;



fprintf(1,'\nCalculating the rotation to be applied to the right and left images in order to bring the epipolar lines aligned with the horizontal scan lines, and in correspondence...\n\n');

R = rodrigues(om);

% Bring the 2 cameras in the same orientation by rotating them "minimally": 
r_r = rodrigues(-om/2);
r_l = r_r';
t = r_r * T;

% Rotate both cameras so as to bring the translation vector in alignment with the (1;0;0) axis:
if abs(t(1)) > abs(t(2)),
    type_stereo = 0;
    uu = [1;0;0]; % Horizontal epipolar lines
else
    type_stereo = 1;
    uu = [0;1;0]; % Vertical epipolar lines
end;
if dot(uu,t)<0,
    uu = -uu; % Swtich side of the vector 
end;
ww = cross(t,uu);
ww = ww/norm(ww);
ww = acos(abs(dot(t,uu))/(norm(t)*norm(uu)))*ww;
R2 = rodrigues(ww);


% Global rotations to be applied to both views:
R_R = R2 * r_r;
R_L = R2 * r_l;


% The resulting rigid motion between the two cameras after image rotations (substitutes of om, R and T):
R_rect = eye(3);
om_rect = zeros(3,1);
T_rect = R_R*T;



% Computation of the *new* intrinsic parameters for both left and right cameras:

% Vertical focal length *MUST* be the same for both images (here, we are trying to find a focal length that retains as much information contained in the original distorted images):
if kc_left(1) < 0,
    fc_y_left_rect = fc_left(2) * (1 + kc_left(1)*(nx^2 + ny^2)/(4*fc_left(2)^2));
else
    fc_y_left_rect = fc_left(2);
end;
if kc_right(1) < 0,
    fc_y_right_rect = fc_right(2) * (1 + kc_right(1)*(nx^2 + ny^2)/(4*fc_right(2)^2));
else
    fc_y_right_rect = fc_right(2);
end;
fc_y_rect = min(fc_y_left_rect,fc_y_right_rect);


% For simplicity, let's pick the same value for the horizontal focal length as the vertical focal length (resulting into square pixels):
fc_left_rect = round([fc_y_rect;fc_y_rect]);
fc_right_rect = round([fc_y_rect;fc_y_rect]);

% Select the new principal points to maximize the visible area in the rectified images

cc_left_rect = [(nx-1)/2;(ny-1)/2] - mean(project_points2([normalize_pixel([0  nx-1 nx-1 0; 0 0 ny-1 ny-1],fc_left,cc_left,kc_left,alpha_c_left);[1 1 1 1]],rodrigues(R_L),zeros(3,1),fc_left_rect,[0;0],zeros(5,1),0),2);
cc_right_rect = [(nx-1)/2;(ny-1)/2] - mean(project_points2([normalize_pixel([0  nx-1 nx-1 0; 0 0 ny-1 ny-1],fc_right,cc_right,kc_right,alpha_c_right);[1 1 1 1]],rodrigues(R_R),zeros(3,1),fc_right_rect,[0;0],zeros(5,1),0),2);


% For simplivity, set the principal points for both cameras to be the average of the two principal points.
if ~type_stereo,
    %-- Horizontal stereo
    cc_y_rect = (cc_left_rect(2) + cc_right_rect(2))/2;
    cc_left_rect = [cc_left_rect(1);cc_y_rect];
    cc_right_rect = [cc_right_rect(1);cc_y_rect];
else
    %-- Vertical stereo
    cc_x_rect = (cc_left_rect(1) + cc_right_rect(1))/2;
    cc_left_rect = [cc_x_rect;cc_left_rect(2)];
    cc_right_rect = [cc_x_rect;cc_right_rect(2)];
end;


% Of course, we do not want any skew or distortion after rectification:
alpha_c_left_rect = 0;
alpha_c_right_rect = 0;
kc_left_rect = zeros(5,1);
kc_right_rect = zeros(5,1);
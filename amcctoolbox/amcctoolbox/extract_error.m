% Color code for each image:

if ~exist('n_ima')|~exist('fc'),
    fprintf(1,'No calibration data available.\n');
    return;
end;

check_active_images;

if n_ima ~=0,
    if ~exist(['ex_' num2str(ind_active(1)) ]),
        fprintf(1,'Need to calibrate before analysing reprojection error. Maybe need to load Calib_Results.mat file.\n');
        return;
    end;
end;


%if ~exist('no_grid'),
no_grid = 0;
%end;

max_err = 0.0;
max_err_img = -1;

max_val = zeros(n_ima,1);

for kk = 1:n_ima,
    if active_images(kk)
        %  Find the length of the error data points vector for the current image we are looking at
        % For each data element, check if its error is bigger
        max_val(kk) = eval(['max(max(abs(ex_' num2str(kk) ')));']);
    else
        max_val(kk) = -1;
    end;
end;

[max_err, max_err_img] = max(max_val);

if(max_err < proj_tol)
    max_err_img = -1;
end
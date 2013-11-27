if ~exist('Calib_Results.mat')
    fprintf(1,'\nCalibration file Calib_Results.mat not found! Looking for alternatives...\n');
    left_available = exist('Calib_Results_left.mat');
    right_available = exist('Calib_Results_right.mat');
    if (left_available && ~right_available)
        fprintf(1,'\nLoading calibration results from Calib_Results_left.mat\n');
        load Calib_Results_left
        return;
    elseif (right_available && ~left_available)
        fprintf(1,'\nLoading calibration results from Calib_Results_right.mat\n');
        load Calib_Results_right
        return;
    elseif (right_available && left_available)
        diff = input('Found both left and right Calib_Results. Please select right or left: ', 's');
        load_string = ['Calib_Results_' diff];
        fprintf(1,['\nLoading calibration results from Calib_Results_' diff '.mat\n']);
        load(load_string);
        return;
    else
        diff = input('Please enter ''Calib_Results_'' suffix (e.g. ''0'': ', 's');
        load_string = ['Calib_Results_' diff];
        fprintf(1,['\nLoading calibration results from Calib_Results_' diff '.mat\n']);
        load(load_string);
        return;
    end;
else
    fprintf(1,'\nLoading calibration results from Calib_Results.mat\n');
end;

load Calib_Results

fprintf(1,'done\n');

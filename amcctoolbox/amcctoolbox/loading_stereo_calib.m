if ~exist('Calib_Results_stereo.mat')
    fprintf(1,'\nCalibration file Calib_Results_stereo.mat not found! Looking for alternatives\n');
    diff = input('Please enter ''Calib_Results_stereo_'' suffix (e.g. ''0_1'': ', 's');
    load_string = ['Calib_Results_stereo_' diff];
    fprintf(1,['\nLoading calibration results from Calib_Results_stereo_' diff '.mat\n']);
    load(load_string);
    return;
else
    fprintf(1,'Loading stereo calibration results from Calib_Results_stereo.mat...\n');
    load('Calib_Results_stereo.mat');
end;


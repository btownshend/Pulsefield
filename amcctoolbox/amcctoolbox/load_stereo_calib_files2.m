
dir('*mat');

fprintf(1,'Loading of the individual left and right camera calibration files\n');

calib_file_name_left = input('Name of the left camera calibration file ([]=Calib_Results_left.mat): ','s');

if isempty(calib_file_name_left),
    calib_file_name_left = 'Calib_Results_left.mat';
end;


calib_file_name_right = input('Name of the right camera calibration file ([]=Calib_Results_right.mat): ','s');

if isempty(calib_file_name_right),
    calib_file_name_right = 'Calib_Results_right.mat';
end;


if (exist(calib_file_name_left)~=2)|(exist(calib_file_name_right)~=2),
    fprintf(1,'Error: left and right calibration files do not exist.\n');
    return;
end;


fprintf(1,'Loading the left camera calibration result file %s...\n',calib_file_name_left);

clear calib_name

load(calib_file_name_left);

fc_left = fc;
cc_left = cc;
kc_left = kc;
alpha_c_left = alpha_c;
KK_left = KK;

center_optim_left = center_optim;
est_alpha_left = est_alpha;
est_dist_left = est_dist;
est_fc_left = est_fc;
est_aspect_ratio_left = est_aspect_ratio;
active_images_left = active_images;


if exist('calib_name')
    calib_name_left = calib_name;
    format_image_left = format_image;
    type_numbering_left = type_numbering;
    image_numbers_left = image_numbers;
    N_slots_left = N_slots;
else
    calib_name_left = '';
    format_image_left = '';
    type_numbering_left = '';
    image_numbers_left = '';
    N_slots_left = '';
end    

for kk = 1:n_ima
    if active_images(kk)
        eval(['omc_left_' num2str(kk) ' = omc_' num2str(kk) ';']);
        eval(['Rc_left_' num2str(kk) ' = Rc_' num2str(kk) ';']);
        eval(['Tc_left_' num2str(kk) ' = Tc_' num2str(kk) ';']);
        s_left{kk}=[eval(['n_sq_x_',num2str(kk)]),eval(['n_sq_y_',num2str(kk)])]+1;
        X_left{kk}=eval(['X_',num2str(kk)]);
        x_left{kk}=eval(['x_',num2str(kk)]);
        omc_left{kk}=eval(['omc_',num2str(kk)]);
        Rc_left{kk}=eval(['Rc_',num2str(kk)]);
        Tc_left{kk}=eval(['Tc_',num2str(kk)]);
        xc_left{kk}=Rc_left{kk}*X_left{kk}+repmat(Tc_left{kk},1,size(X_left{kk},2));
        planes_left(:,kk)=Rc_left{kk}(:,3)*Rc_left{kk}(:,3)'*Tc_left{kk};
   end
end




fprintf(1,'Loading the right camera calibration result file %s...\n',calib_file_name_right);

clear calib_name

load(calib_file_name_right);

fc_right = fc;
cc_right = cc;
kc_right = kc;
alpha_c_right = alpha_c;
KK_right = KK;

if exist('calib_name')
    calib_name_right = calib_name;
    format_image_right = format_image;
    type_numbering_right = type_numbering;
    image_numbers_right = image_numbers;
    N_slots_right = N_slots;
else
    calib_name_right = '';
    format_image_right = '';
    type_numbering_right = '';
    image_numbers_right = '';
    N_slots_right = '';
end

center_optim_right = center_optim;
est_alpha_right = est_alpha;
est_dist_right = est_dist;
est_fc_right = est_fc;
est_aspect_ratio_right = est_aspect_ratio;
active_images_right = active_images;

for kk = 1:n_ima
    if active_images(kk)
        eval(['omc_right_' num2str(kk) ' = omc_' num2str(kk) ';']);
        eval(['Rc_right_' num2str(kk) ' = Rc_' num2str(kk) ';']);
        eval(['Tc_right_' num2str(kk) ' = Tc_' num2str(kk) ';']);
        s_right{kk}=[eval(['n_sq_x_',num2str(kk)]),eval(['n_sq_y_',num2str(kk)])]+1;       
        X_right{kk}=eval(['X_',num2str(kk)]);
        x_right{kk}=eval(['x_',num2str(kk)]);
        omc_right{kk}=eval(['omc_',num2str(kk)]);
        Rc_right{kk}=eval(['Rc_',num2str(kk)]);
        Tc_right{kk}=eval(['Tc_',num2str(kk)]);
        xc_right{kk}=Rc_right{kk}*X_right{kk}+repmat(Tc_right{kk},1,size(X_right{kk},2));
        planes_right(:,kk)=Rc_right{kk}(:,3)*Rc_right{kk}(:,3)'*Tc_right{kk};
   end
end

% reset pointsmatched
pointsmatched=0;


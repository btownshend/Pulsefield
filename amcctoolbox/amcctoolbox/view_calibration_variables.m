% Print out all the calibration variables

fprintf(1,'\nStereo calibration variables:\n');

fprintf(1,'\n\nVariables for left camera:\n\n');
fprintf(1,'center_optim_left = %d\n',center_optim_left);
fprintf(1,'est_aspect_ratio_left = %d\n',est_aspect_ratio_left);
fprintf(1,'est_alpha_left = %d\n',est_alpha_left);
fprintf(1,'est_dist_left = [ %d   %d   %d   %d  %d ] \n',est_dist_left);   
fprintf(1,'recompute_intrinsic_left = %d\n',recompute_intrinsic_left);

fprintf(1,'\n\nVariables for right camera:\n\n');
fprintf(1,'center_optim_right = %d\n',center_optim_right);
fprintf(1,'est_aspect_ratio_right = %d\n',est_aspect_ratio_right);
fprintf(1,'est_alpha_right = %d\n',est_alpha_right);
fprintf(1,'est_dist_right = [ %d   %d   %d   %d  %d ] \n',est_dist_right);   
fprintf(1,'recompute_intrinsic_right = %d\n',recompute_intrinsic_right);
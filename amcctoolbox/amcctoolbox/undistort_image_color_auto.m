%%% INPUT THE IMAGE FILE NAME:

if ~exist('fc')|~exist('cc')|~exist('kc')|~exist('alpha_c')
    fprintf(1,'No intrinsic camera parameters available.\n');
    return
end

if ~exist('rotate_img')
    rotate_img = false;
end

clear I I2;

KK = [fc(1) alpha_c*fc(1) cc(1);0 fc(2) cc(2) ; 0 0 1];

disp('Program that corrects distorted images');
disp('The intrinsic camera parameters are assumed to be known (previously computed)');

fprintf(1,'\n')

quest = 1; %input('Do you want to undistort all the calibration images ([],0) or a new image (1)? ');

if isempty(quest)
    quest = 0;
end

if ~exist('ima_name')
    ima_name = input('Location of image 0 : ','s');
end
image_name = ima_name(1:size(ima_name,2) - 3)
format_image2 = ima_name(size(ima_name,2) - 2:size(ima_name,2))

%%% READ IN IMAGE:
I_t = double(imread(ima_name));
%%% rotate it if necessary
if (rotate_img)
    I = imrotate(I_t,180);
else
    I = I_t;
end
if (size(I,1)>ny)|(size(I,2)>nx)
    I = I(1:ny,1:nx);
end



%% SHOW THE ORIGINAL IMAGE:
figure(2);
image(uint8(I));
title('Original image (with distortion) - Stored in array I')
drawnow;

%% UNDISTORT THE IMAGE:
if(size(I,3) == 1)
    %grey
    fprintf(1,'Computing the undistorted greyscale image...')
    [Ipart_1] = rect(I(:,:,1),eye(3),fc,cc,kc,alpha_c,KK,fisheye);
    I2(:,:,1) = Ipart_1;  
else 
    %colour
    fprintf(1,'Computing the undistorted colour image...')
    [Ipart_1] = rect(I(:,:,1),eye(3),fc,cc,kc,alpha_c,KK,fisheye);
    [Ipart_2] = rect(I(:,:,2),eye(3),fc,cc,kc,alpha_c,KK,fisheye);
    [Ipart_3] = rect(I(:,:,3),eye(3),fc,cc,kc,alpha_c,KK,fisheye);

    I2 = ones(ny, nx,3);
    I2(:,:,1) = Ipart_1;
    I2(:,:,2) = Ipart_2;
    I2(:,:,3) = Ipart_3;
end

fprintf(1,'done\n')

figure(3);
image(uint8(I2));


title('Undistorted image - Stored in array I2')
drawnow;

%% SAVE THE IMAGE IN FILE:
if ~exist('ima_out_name','var')
    ima_out_name = input('Location of output image : ','s');
end

% Make sure the output directory exists
if (~exist([output_dir],'dir'))
    display(['Making a calibration directory at ''' output_dir '...']);
    mkdir(output_dir) % Make the directory if it doesn't already exist
end

fprintf(1,['Saving undistorted image under ' ima_out_name '...'])
imwrite(uint8(round(I2)),ima_out_name,format_image2);
fprintf(1,'done\n')
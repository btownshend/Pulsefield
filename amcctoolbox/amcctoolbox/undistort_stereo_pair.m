%%% INPUT THE IMAGE FILE NAME:

if ~exist('fc_left')|~exist('cc_left')|~exist('kc_left')|~exist('alpha_c_left'),
   fprintf(1,'No intrinsic camera parameters available for left camera.\n');
   return;
end;

if ~exist('fc_right')|~exist('cc_right')|~exist('kc_right')|~exist('alpha_c_right'),
   fprintf(1,'No intrinsic camera parameters available for right camera.\n');
   return;
end;

KK_left = [fc_left(1) alpha_c_left*fc_left(1) cc_left(1);0 fc_left(2) cc_left(2) ; 0 0 1];
KK_right = [fc_right(1) alpha_c_right*fc_right(1) cc_right(1);0 fc_right(2) cc_right(2) ; 0 0 1];

disp('Program that undistorts images');
disp('The intrinsic camera parameters are assumed to be known (previously computed)');

fprintf(1,'\n');

quest = input('Do you want to undistort a set of the calibration images ([],0) or a new image (1)? ');

if isempty(quest),
   quest = 0;
end;

if (~exist('input_dir','var'))
    input_dir = '';
end

if ~quest,

    if ~isempty(calib_name_left) && ~isempty(calib_name_right),
    
        fprintf(1,'Undistorting all the images (this should be fast)...\n\n');

        % Rectify all the images: (This is fastest way to proceed: precompute the set of image indices, and blending coefficients before actual image warping!)

        for kk = find(active_images);
       
            % Left image:

            I = load_image(kk,calib_name_left,format_image_left,type_numbering_left,image_numbers_left,N_slots_left, input_dir);
 
            fprintf(1,'Computing the undistorted left image...')
    
            [I2] = rect(I,eye(3),fc_left,cc_left,kc_left,alpha_c_left,KK_left,fisheye);
    
            fprintf(1,'done\n');

            write_image(I2,kk,[calib_name_left '_undistorted'],format_image_left,type_numbering_left,image_numbers_left,N_slots_left, [input_dir 'undistorted/']),

            fprintf(1,'\n');

            % Right image:

            I = load_image(kk,calib_name_right,format_image_right,type_numbering_right,image_numbers_right,N_slots_right, input_dir);

            fprintf(1,'Computing the undistorted right image...\n');

            [I2] = rect(I,eye(3),fc_right,cc_right,kc_right,alpha_c_right,KK_right,fisheye);

            write_image(I2,kk,[calib_name_right '_undistorted'],format_image_right,type_numbering_right,image_numbers_right,N_slots_right, [input_dir 'undistorted/']);

            fprintf(1,'\n');
        
        end;
    
    end;
   
   fprintf(1,'done\n');

else
    
    dir;
    fprintf(1,'\n');
    
    image_name = input('Image name (full name without extension): ','s');
    
    format_image2 = '0';
    
    while format_image2 == '0',
        
        format_image2 =  input('Image format: ([]=''r''=''ras'', ''b''=''bmp'', ''t''=''tif'', ''p''=''pgm'', ''j''=''jpg'', ''m''=''ppm'') ','s');
        
        if isempty(format_image2),
            format_image2 = 'ras';
        end;
        
        if lower(format_image2(1)) == 'm',
            format_image2 = 'ppm';
        else
            if lower(format_image2(1)) == 'b',
                format_image2 = 'bmp';
            else
                if lower(format_image2(1)) == 't',
                    format_image2 = 'tif';
                else
                    if lower(format_image2(1)) == 'p',
                        format_image2 = 'pgm';
                    else
                        if lower(format_image2(1)) == 'j',
                            format_image2 = 'jpg';
                        else
                            if lower(format_image2(1)) == 'r',
                                format_image2 = 'ras';
                            else  
                                disp('Invalid image format');
                                format_image2 = '0'; % Ask for format once again
                            end;
                        end;
                    end;
                end;
            end;
        end;
    end;
    
    ima_name = [image_name '.' format_image2];
    
    
    %%% READ IN IMAGE:
    I = double(imread(ima_name));
    
    if size(I,3)>1,
        I = I(:,:,2);
    end;
    
    
    if (size(I,1)>ny)|(size(I,2)>nx),
        I = I(1:ny,1:nx);
    end;
    
    
    %% SHOW THE ORIGINAL IMAGE:
    
    figure(2);
    image(I);
    colormap(gray(256));
    title('Original image (with distortion) - Stored in array I');
    drawnow;
    
    
    %% UNDISTORT THE IMAGE:
    
    fprintf(1,'Computing the undistorted image...')
    
    [I2] = rect(I,eye(3),fc,cc,kc,alpha_c,KK,fisheye);
    
    fprintf(1,'done\n');
    
    figure(3);
    image(I2);
    colormap(gray(256));
    title('Undistorted image - Stored in array I2');
    drawnow;
    
    
    %% SAVE THE IMAGE IN FILE:
    
    ima_name2 = [image_name '_rect.' format_image2];
    
    fprintf(1,['Saving undistorted image under ' ima_name2 '...']);
    
    imwrite(uint8(round(I2)),gray(256),ima_name2,format_image2);
    
    fprintf(1,'done\n');
    
end;

%%% INPUT THE IMAGE FILE NAME:

% Force the input_dir to the current directory if not a variable
if (~exist('input_dir','var'))
    input_dir = './';
end

path_exists = 0;
if (~exist('undistorted_path') || isempty(undistorted_path))
    % Get the output name from the user
    path_exists = 1;
    undistorted_path = input('Enter the output undistorted image directory (or specify it with ''undistorted_path='', leave empty for the default: ''undistorted/'') ','s');
    if(isempty(undistorted_path))
        undistorted_path = [output_dir 'undistorted/'];
    end
end

% Make sure the output directory exists
if (~exist(undistorted_path,'dir'))
    display(['Making an undistorted image directory at ''' undistorted_path  '''...']);
    mkdir(undistorted_path) % Make the directory if it doesn't already exist
end

KK = [fc(1) alpha_c*fc(1) cc(1);0 fc(2) cc(2) ; 0 0 1];

disp('Program that undistorts images');
disp('The intrinsic camera parameters are assumed to be known (previously computed)');

fprintf(1,'\n');

quest = input('Do you want to undistort a calibration image ([],0) or a new image (1)? ');

if isempty(quest),
   quest = 0;
end;

if ~quest,
    
	%if ~exist(['I_' num2str(ind_active(1))]),
   	%ima_read_calib;
    %end;
    
    if ~exist('dont_ask'),
        dont_ask = 0;
    end;


    if (~dont_ask)&(length(ind_active)>1),
       ima_numbers = input('Number(s) of calibration image(s) to undistort ([] = all images) = ');
    else
       ima_numbers = [];
    end;


    if isempty(ima_numbers),
       ima_proc = 1:n_ima;
    else
       ima_proc = ima_numbers;
    end;

   check_active_images;   
   
   format_image2 = format_image;
   if format_image2(1) == 'j',
      format_image2 = 'bmp';
   end;
   
   
   for ii = 1:size(ima_proc),
       
       kk = ima_proc(ii);
       
       if ~type_numbering,   
           number_ext =  num2str(image_numbers(kk));
       else
           number_ext = sprintf(['%.' num2str(N_slots) 'd'],image_numbers(kk));
       end;
       
       ima_name = [input_dir calib_name  number_ext '.' format_image];
       
       
       if ~exist(ima_name,'file'),
           
           fprintf(1,'Image %s not found!!!\n',ima_name);
           
       else
           
           fprintf(1,'Loading image %s...\n',ima_name);
           
           I = double(imread(ima_name));
           
           if size(I,3)>1,
               I = 0.299 * I(:,:,1) + 0.5870 * I(:,:,2) + 0.114 * I(:,:,3);
           end;
           
           [I2] = rect(I,eye(3),fc,cc,kc,alpha_c,KK,fisheye);
           
           if ~type_numbering,   
               number_ext =  num2str(image_numbers(kk));
           else
               number_ext = sprintf(['%.' num2str(N_slots) 'd'],image_numbers(kk));
           end;
           
           ima_name2 = [undistorted_path calib_name '_undistorted' number_ext '.' format_image2];
           
           fprintf(1,['Saving undistorted image under ' ima_name2 '...\n']);
           
           imwrite(uint8(round(I2)),gray(256),ima_name2,format_image2);
           
           
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
    
    if format_image2(1) == 'p',
        if format_image2(2) == 'p',
            I = double(loadppm(ima_name));
        else
            I = double(loadpgm(ima_name));
        end;
    else
        if format_image2(1) == 'r',
            I = readras(ima_name);
        else
            I = double(imread(ima_name));
        end;
    end;
    
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

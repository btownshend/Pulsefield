function I = load_image(kk , calib_name , format_image , type_numbering , image_numbers , N_slots, input_dir, rot_flag),

if (~exist('input_dir','var'))
    input_dir = '';
end

if (~exist('rot_flag','var'))
    rot_flag = 0;
end

if ~type_numbering,   
    number_ext =  num2str(image_numbers(kk));
else
    number_ext = sprintf(['%.' num2str(N_slots) 'd'],image_numbers(kk));
end;


ima_name = [input_dir calib_name  number_ext '.' format_image];

if ~exist(ima_name),
    
    fprintf(1,'Image %s not found!!!\n',ima_name);
    I = NaN;
    
else
    
    fprintf(1,'Loading image %s...\n',ima_name);
    
    I = double(imread(ima_name));
    
    % rotate if the flag is true
    if rot_flag
        display('Rotating image 180 degrees...')
        I = imrotate(I,180);
    end
    
    if size(I,3)>1,
        I = 0.299 * I(:,:,1) + 0.5870 * I(:,:,2) + 0.114 * I(:,:,3);
    end;
    
end;

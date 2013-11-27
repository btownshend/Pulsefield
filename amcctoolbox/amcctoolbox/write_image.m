function ima_name2 = write_image(I, kk , calib_name , format_image , type_numbering , image_numbers , N_slots, output_dir),

if (~exist('output_dir','var'))
    output_dir = '';
end

if format_image(1) == 'j',
    format_image = 'bmp';
end;


% Make sure the output directory exists
if (~exist(output_dir,'dir'))
    display(['Making an output image directory at ''' output_dir  '''...']);
    mkdir(output_dir) % Make the directory if it doesn't already exist
end

if ~type_numbering,   
    number_ext =  num2str(image_numbers(kk));
else
    number_ext = sprintf(['%.' num2str(N_slots) 'd'],image_numbers(kk));
end;

ima_name2 = [output_dir calib_name  number_ext '.' format_image];

fprintf(1,['Saving image under ' ima_name2 '...\n']);

if format_image(1) == 'p',
    if format_image(2) == 'p',
        saveppm(ima_name2,uint8(round(I)));
    else
        savepgm(ima_name2,uint8(round(I)));
    end;
else
    if format_image(1) == 'r',
        writeras(ima_name2,round(I),gray(256));
    else
        imwrite(uint8(round(I)),gray(256),ima_name2,format_image);
    end;
end;

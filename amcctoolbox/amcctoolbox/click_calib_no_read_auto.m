% Copyright (c) Michael Warren 2013

% This file is part of the AMMCC Toolbox.

% The AMMCC Toolbox is free software: you can redistribute it and/or modify
% it under the terms of the GNU Lesser General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
% 
% The AMMCC Toolbox is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Lesser General Public License for more details.
% 
% You should have received a copy of the GNU Lesser General Public License
% along with the AMMCC Toolbox.  If not, see <http://www.gnu.org/licenses/>.


%if exist('images_read');
%   active_images = active_images & images_read;
%end;

dont_ask = true;

var2fix = 'dX_default';

fixvariable;

var2fix = 'dY_default';

fixvariable;

var2fix = 'map';

fixvariable;


% Set the variable if it doesn't exist
if ~exist('fisheye','var')
    fisheye = false;
end

if ~exist('n_ima'),
    data_calib_no_read_auto;
end;

if ~exist('rot_flag'),
    rot_flag = false;
end;


check_active_images;

% Step used to clean the memory if a previous atttempt has been made to read the entire set of images into memory:
for kk = 1:n_ima,
    if (exist(['I_' num2str(kk)])==1),
        clear(['I_' num2str(kk)]);
    end;
end;

fprintf(1,'\nExtraction of the grid corners on the images\n');


if (exist('map')~=1), map = gray(256); end;


%disp('WARNING!!! Do not forget to change dX_default and dY_default in click_calib.m!!!')

if exist('dX'),
    dX_default = dX;
end;

if exist('dY'),
    dY_default = dY;
end;

if exist('n_sq_x'),
    n_sq_x_default = n_sq_x;
end;

if exist('n_sq_y'),
    n_sq_y_default = n_sq_y;
end;


if ~exist('dX_default')|~exist('dY_default');
    
    % Setup of JY - 3D calibration rig at Intel (new at Intel) - use units in mm to match Zhang
    dX_default = 30;
    dY_default = 30;
    
    % Setup of JY - 3D calibration rig at Google - use units in mm to match Zhang
    dX_default = 0.1524; %100;
    dY_default = 0.1524; %100;
end;


if ~exist('n_sq_x_default')|~exist('n_sq_y_default'),
    n_sq_x_default = 10;
    n_sq_y_default = 10;
end;


if ~exist('wintx_default')|~exist('winty_default'),
    if ~exist('nx'),
        wintx_default = 30;
        winty_default = wintx_default;
        clear wintx winty
    else
        wintx_default = max(round(nx/128),round(ny/96));
        winty_default = wintx_default;
        clear wintx winty
    end;
end;


if ~exist('wintx') | ~exist('winty'),
    clear_windows; % Clear all the window sizes (to re-initiate)
end;



if ~exist('dont_ask'),
    dont_ask = 0;
end;


if ~dont_ask,
    ima_numbers = input('Number(s) of image(s) to process ([] = all images) = ');
else
    ima_numbers = [];
end;

if isempty(ima_numbers),
    ima_proc = 1:n_ima;
else
    ima_proc = ima_numbers;
end;


% Useful option to add images:
kk_first = ima_proc(1); %input('Start image number ([]=1=first): ');

%if isempty(kk_first), kk_first = 1; end;


% if exist(['wintx_' num2str(kk_first)]),
%     
%     eval(['wintxkk = wintx_' num2str(kk_first) ';']);
%     
%     if isempty(wintxkk) | isnan(wintxkk),
%         
%         disp('Window size for corner finder (wintx and winty):');
%         wintx = input(['wintx ([] = ' num2str(wintx_default) ') = ']);
%         if isempty(wintx), wintx = wintx_default; end;
%         wintx = round(wintx);
%         winty = input(['winty ([] = ' num2str(winty_default) ') = ']);
%         if isempty(winty), winty = winty_default; end;
%         winty = round(winty);
%         
%         fprintf(1,'Window size = %dx%d\n',2*wintx+1,2*winty+1);
%         
%     end;
%     
% else
%     
%     disp('Window size for corner finder (wintx and winty):');
%     wintx = input(['wintx ([] = ' num2str(wintx_default) ') = ']);
%     if isempty(wintx), wintx = wintx_default; end;
%     wintx = round(wintx);
%     winty = input(['winty ([] = ' num2str(winty_default) ') = ']);
%     if isempty(winty), winty = winty_default; end;
%     winty = round(winty);
%     
%     fprintf(1,'Window size = %dx%d\n',2*wintx+1,2*winty+1);
%     
% end;
% 
% if ~dont_ask,
%     fprintf(1,'Do you want to use the automatic square counting mechanism (0=[]=default)\n');
%     manual_squares = input('  or do you always want to enter the number of squares manually (1,other)? ');
%     if isempty(manual_squares),
%         manual_squares = 0;
%     else
%         manual_squares = ~~manual_squares;
%     end;
% else
%     manual_squares = 0;
% end;

check_nums = [];
ll = 0;

for kk = ima_proc,
    

    
    if ~type_numbering,   
        number_ext =  num2str(image_numbers(kk));
    else
        number_ext = sprintf(['%.' num2str(N_slots) 'd'],image_numbers(kk));
    end;
    
    ima_name = [input_dir calib_name  number_ext '.' format_image];
    
    
    if exist(ima_name,'file'),
        
        % increment the counter
        ll = ll + 1;
        
        fprintf(1,['\n**************** Processing image %d (%d of %d) for base name ''' calib_name ''' ****************\n'],kk,ll,size(ind_active,2));

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
        
        [ny,nx,junk] = size(I);
        Wcal = nx; % to avoid errors later
        Hcal = ny; % to avoid errors later
        
        click_ima_calib_auto;
        %click_ima_calib_no_read_auto;
                
        % Fail Mode
        if exist(['extfail_',num2str(kk)],'var')
            if eval(['extfail_',num2str(kk)])==true
                eval(['extfail_',num2str(kk),'=false;']);%reset
                fprintf('\nImage %d Failed and Suppressed\n',kk);
                continue;
            end
        end

        % Suspicious Mode
        if exist(['sus_',num2str(kk)],'var')
            if eval(['sus_',num2str(kk)])
                eval(['sus_',num2str(kk),'=0;'])
                %I=eval(['I_',num2str(kk)]);
                %figure(2);
                image(I); colormap(map); hold on;
                grid_pts_mat=eval(['grid_pts_mat_',num2str(kk)]);
                %plot(grid_pts_mat(:,:,2),grid_pts_mat(:,:,1),'+');
                fprintf('\nPlease Check Validity of Corners on Image %d. Suppress If necessary\n',kk);
                check_nums = [check_nums kk];
                %pause;
            end
        end
        active_images(kk) = 1;
        
    else
        eval(['dX_' num2str(kk) ' = NaN;']);
        eval(['dY_' num2str(kk) ' = NaN;']);  
        
        eval(['wintx_' num2str(kk) ' = NaN;']);
        eval(['winty_' num2str(kk) ' = NaN;']);
        
        eval(['x_' num2str(kk) ' = NaN*ones(2,1);']);
        eval(['X_' num2str(kk) ' = NaN*ones(3,1);']);
        
        eval(['n_sq_x_' num2str(kk) ' = NaN;']);
        eval(['n_sq_y_' num2str(kk) ' = NaN;']);
    end;

end;

fprintf('\nPlease check validity of corners on the following image numbers (Suppress If necessary):\n');
check_nums
check_active_images;


% Fix potential non-existing variables:

for kk = 1:n_ima,
    if ~exist(['x_' num2str(kk)]),
        eval(['dX_' num2str(kk) ' = NaN;']);
        eval(['dY_' num2str(kk) ' = NaN;']);  
        
        eval(['x_' num2str(kk) ' = NaN*ones(2,1);']);
        eval(['X_' num2str(kk) ' = NaN*ones(3,1);']);
        
        eval(['n_sq_x_' num2str(kk) ' = NaN;']);
        eval(['n_sq_y_' num2str(kk) ' = NaN;']);
    end;
    
    if ~exist(['wintx_' num2str(kk)]) | ~exist(['winty_' num2str(kk)]),
        
        eval(['wintx_' num2str(kk) ' = NaN;']);
        eval(['winty_' num2str(kk) ' = NaN;']);
        
    end;
    if ~exist('wintx')
        wintx=NaN;
        winty=NaN;
    end
end;


string_save = 'save calib_data active_images ind_active wintx winty n_ima type_numbering N_slots first_num image_numbers format_image calib_name Hcal Wcal nx ny map dX_default dY_default dX dY';

for kk = 1:n_ima,
    string_save = [string_save ' X_' num2str(kk) ' x_' num2str(kk) ' n_sq_x_' num2str(kk) ' n_sq_y_' num2str(kk) ' wintx_' num2str(kk) ' winty_' num2str(kk) ' dX_' num2str(kk) ' dY_' num2str(kk)];
end;

orig_dir = pwd;

% Make sure the output directory exists
if (~exist([output_dir],'dir'))
    display(['Making a calibration directory at ''' output_dir '...']);
    mkdir(output_dir) % Make the directory if it doesn't already exist
end

cd(output_dir);
eval(string_save);
cd(orig_dir);

disp('done');

return;

%go_calib_optim;

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




% Color code for each image:

if ~exist('n_ima')|~exist('fc'),
    fprintf(1,'No calibration data available.\n');
    return;
end;

check_active_images;

if n_ima ~=0,
    if ~exist(['ex_' num2str(ind_active(1)) ]),
        fprintf(1,'Need to calibrate before analysing reprojection error. Maybe need to load Calib_Results.mat file.\n');
        return;
    end;
end;


%if ~exist('no_grid'),
no_grid = 0;
%end;

max_err = 0.0;
max_err_img = -1;

% Make the suppression vector empty
ima_numbers = [];
num_per_iter = 10;
err = [];

for kk = 1:n_ima,
    display(['Looking at image ' num2str(kk) ' of ' num2str(n_ima)]);
    if active_images(kk) & eval(['~isnan(y_' num2str(kk) '(1,1))']),
        if exist(['y_' num2str(kk)]),   
            if ~no_grid,
                eval(['XX_kk = X_' num2str(kk) ';']);
                N_kk = size(XX_kk,2);
                
                if ~exist(['n_sq_x_' num2str(kk)]),
                    no_grid = 1;
                end;
                
                if ~no_grid,
                    eval(['n_sq_x = n_sq_x_' num2str(kk) ';']);
                    eval(['n_sq_y = n_sq_y_' num2str(kk) ';']);
                    if (N_kk ~= ((n_sq_x+1)*(n_sq_y+1))),
                        no_grid = 1;
                    end;
                end;
            end;
            %  Find the length of the error data points vector for the current image we are looking at
            eval(['len_ex = size(ex_' num2str(kk) ');']);
            % For each data element, check if its error is bigger
            for ii=1:1:len_ex(2)
                eval(['x_val = abs(ex_' num2str(kk) '(1,ii));']); % x value
                eval(['y_val = abs(ex_' num2str(kk) '(2,ii));']); % y value
                if (x_val > max_err && x_val > proj_tol)
                    max_err = x_val;
                    max_err_img = kk;
                elseif (y_val > max_err && y_val > proj_tol)
                    max_err = y_val;
                    max_err_img = kk;
                end
                
                % Definitely throw away images with a large error
                if (x_val > 2*proj_tol)
                    ima_numbers = [ima_numbers max_err_img];
                elseif (y_val > 2*proj_tol)
                    ima_numbers = [ima_numbers max_err_img];
                end
                
                if (max_err > 2*proj_tol)
                    % Break if we have hit num_per_iter images
                    ima_numbers_sz = size(ima_numbers);
                    if (ima_numbers_sz(1) > num_per_iter)
                        return;
                    end
                end
            end
            %eval(['plot(ex_' num2str(kk) '(1,:),ex_' num2str(kk) '(2,:),''' colors(rem(kk-1,6)+1) '+'');']);
            %plot(ex_20(1,:),ex_20(2,:),'r+');
            %hold on;
        end;
    end;
end;

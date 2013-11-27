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

colors = 'brgkcm';


figure(5);
m_ = [];
for kk = 1:n_ima,
    if exist(['y_' num2str(kk)])
        if active_images(kk) & eval(['~isnan(y_' num2str(kk) '(1,1))'])
            if ~no_grid
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
            
            
            % Make a list of image reprojection errors
            eval(['xy_kk = ex_' num2str(kk) '(1:2,:);']);
            xy_sz = size(xy_kk);
            m_sz = size(m_);
            m_(1:2,m_sz(2)+1:m_sz(2)+xy_sz(2)) = xy_kk;
            
            % Add the pixel locations to the list
            %eval(['x_' num2str(kk) '(1:2,:)''']);
            eval(['m_(3:4,m_sz(2)+1:m_sz(2)+xy_sz(2)) = x_' num2str(kk) '(1:2,:);']);
            %m_(3:4,m_sz(2)+1:m_sz(2)+xy_sz(2))
            %eval(['plot(x_' num2str(kk) '(1,:)+1,x_' num2str(kk) '(2,:)+1,''r+'');']);
            
            
            % Extract the Z depth of the points
            if (exist(['X_' num2str(kk)]) & exist(['omc_' num2str(kk)]))
                eval(['XX_kk = X_' num2str(kk) ';']);
                if ~isnan(XX_kk(1,1)),
                    eval(['omc_kk = omc_' num2str(kk) ';']);
                    eval(['Tc_kk = Tc_' num2str(kk) ';']);
                    N_kk = size(XX_kk,2);
                    if ~exist(['n_sq_x_' num2str(kk)]),
                        no_grid = 1;
                    else
                        eval(['n_sq_x = n_sq_x_' num2str(kk) ';']);
                        if isnan(n_sq_x(1)),
                            no_grid = 1;
                        end;  
                    end;
                    if ~no_grid
                        eval(['n_sq_x = n_sq_x_' num2str(kk) ';']);
                        eval(['n_sq_y = n_sq_y_' num2str(kk) ';']);
                        if (N_kk ~= ((n_sq_x+1)*(n_sq_y+1))),
                            no_grid = 1;
                        end;
                    end;

                    if ~isnan(omc_kk(1,1)),

                       R_kk = rodrigues(omc_kk);

                       YY_kk = R_kk * XX_kk + Tc_kk * ones(1,length(XX_kk));

                       uu = [-dX;-dY;0]/2;
                       uu = R_kk * uu + Tc_kk; 

                       if ~no_grid,
                          %YYx = zeros(n_sq_x+1,n_sq_y+1);
                          %YYy = zeros(n_sq_x+1,n_sq_y+1);
                          %YYz = zeros(n_sq_x+1,n_sq_y+1);

                          %YYx(:) = YY_kk(1,:);
                          %YYy(:) = YY_kk(2,:);
                          %YYz(:) = YY_kk(3,:);
                          m_(5,m_sz(2)+1:m_sz(2)+xy_sz(2)) = YY_kk(3,:);

                       end;
                    end;
                end;      
            end;
        end;
    end;
end;
m_';
size(m_');
hist3(m_(1:2,:)',[30 30]);
m_sz = size(m_);

max_dist = max(m_(5,:));
% Split the error values into  bins
bin_sz = 100;
% Clear any values that existed previously
for jj=0:bin_sz:max_dist
    eval(['clear m_' num2str(jj) ';'])
end
% Split the data
for ii=1:1:m_sz(2)
    for jj=0:bin_sz:max_dist
        if (m_(5,ii) > jj && m_(5,ii) < jj+bin_sz)
            %eval('m_', num2str(jj), '_sz = size(m_', num2str(jj));
            if ~exist(['m_' num2str(jj)],'var')
                %eval(['m_' num2str(jj) '= [0 0 0]']);
                %num2str(m_(1:3,ii)')
                eval(['m_' num2str(jj) ' = m_(:,ii);' ]);
            else
                %eval(['m_' num2str(jj)]);
                %m_str = ['m_' num2str(jj) ' = [m_' num2str(jj) ' = m_(:,ii) ];'];
                eval(['m_' num2str(jj) ' = [m_' num2str(jj) ' m_(:,ii) ];']);
            end
        end
    end
end

R_list = [];
% Calculate the measurement noise covariance matrix
for jj=0:bin_sz:max_dist
    if exist(['m_' num2str(jj)],'var')
        eval(['m_nn = m_' num2str(jj) ';']);
        sz_m_nn = size(m_nn);
        XY_mean = [0 0];
        XY_mean(1,1) = mean(m_nn(1,:));
        XY_mean(1,2) = mean(m_nn(2,:));
        R_nn_sum = 0;
        for kk=1:1:sz_m_nn(2)
            m_nn(1:2,kk);
            XY_mean;
            R_nn_sum = R_nn_sum + (m_nn(1:2,kk) - XY_mean')*(m_nn(1:2,kk) - XY_mean')';
        end
        eval(['R_' num2str(jj) ' = (1/sz_m_nn(2))*R_nn_sum']);
        eval(['R_list = [R_list R_' num2str(jj) '(1,1)];']);
    end
end

num_successful = 0;
num_total = 0;
% Geary's test
for jj=0:bin_sz:max_dist
    if exist(['m_' num2str(jj)],'var')
        display(jj);
        eval(['r = gtest(m_' num2str(jj) '(1,:)'',alpha)']);
        if r
            num_successful = num_successful + 1;
        end
        num_total = num_total + 1;
    end
end

num_successful
num_total
ratio = num_successful/num_total
    
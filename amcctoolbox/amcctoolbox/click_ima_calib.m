% Cleaned-up version of init_calib.m



fprintf(1,'\nProcessing image %d...\n',kk);

eval(['I = I_' num2str(kk) ';']);


try
    autocrnrst=findcorners(I);
catch
    active_images(kk) = 0;
    eval(['dX_' num2str(kk) ' = NaN;']);
    eval(['dY_' num2str(kk) ' = NaN;']);  

    eval(['wintx_' num2str(kk) ' = NaN;']);
    eval(['winty_' num2str(kk) ' = NaN;']);

    eval(['x_' num2str(kk) ' = NaN*ones(2,1);']);
    eval(['X_' num2str(kk) ' = NaN*ones(3,1);']);

    eval(['n_sq_x_' num2str(kk) ' = NaN;']);
    eval(['n_sq_y_' num2str(kk) ' = NaN;']);
    eval(['extfail_' num2str(kk) ' = true;']);
    fprintf('\nFailed\n');
    return;
end

if autocrnrst.fail==true
    active_images(kk) = 0;
    eval(['dX_' num2str(kk) ' = NaN;']);
    eval(['dY_' num2str(kk) ' = NaN;']);  

    eval(['wintx_' num2str(kk) ' = NaN;']);
    eval(['winty_' num2str(kk) ' = NaN;']);

    eval(['x_' num2str(kk) ' = NaN*ones(2,1);']);
    eval(['X_' num2str(kk) ' = NaN*ones(3,1);']);

    eval(['n_sq_x_' num2str(kk) ' = NaN;']);
    eval(['n_sq_y_' num2str(kk) ' = NaN;']);
    eval(['extfail_' num2str(kk) ' = true;']);
    fprintf('Failed :(\n');
    return;
end

grid_pts_mat=autocrnrst.grid;
sus=autocrnrst.suspicious;
if sus
    fprintf('Suspicious...\n');
else
    fprintf('Success!!\n');
end
wintx=autocrnrst.win;
winty=autocrnrst.win;

n_sq_x=size(grid_pts_mat,1)-1;
n_sq_y=size(grid_pts_mat,2)-1;


n_sq_x_default = n_sq_x;
n_sq_y_default = n_sq_y;
    


Np = (n_sq_x+1)*(n_sq_y+1);



nopts_mat=size(grid_pts_mat,1)*size(grid_pts_mat,2);
grid_pts=reshape(grid_pts_mat(:,:,1),1,nopts_mat);
grid_pts=[reshape(grid_pts_mat(:,:,2),1,nopts_mat);grid_pts];




%save all_corners x y grid_pts

grid_pts = grid_pts - 1; % subtract 1 to bring the origin to (0,0) instead of (1,1) in matlab (not necessary in C)



ind_corners = [1 n_sq_x+1 (n_sq_x+1)*n_sq_y+1 (n_sq_x+1)*(n_sq_y+1)]; % index of the 4 corners
ind_orig = (n_sq_x+1)*n_sq_y + 1;
xorig = grid_pts(1,ind_orig);
yorig = grid_pts(2,ind_orig);
dxpos = mean([grid_pts(:,ind_orig) grid_pts(:,ind_orig+1)]');
dypos = mean([grid_pts(:,ind_orig) grid_pts(:,ind_orig-n_sq_x-1)]');


x_box_kk = [grid_pts(1,:)-(wintx+.5);grid_pts(1,:)+(wintx+.5);grid_pts(1,:)+(wintx+.5);grid_pts(1,:)-(wintx+.5);grid_pts(1,:)-(wintx+.5)];
y_box_kk = [grid_pts(2,:)-(winty+.5);grid_pts(2,:)-(winty+.5);grid_pts(2,:)+(winty+.5);grid_pts(2,:)+(winty+.5);grid_pts(2,:)-(winty+.5)];

delta=30;

% figure(3);
% image(I); colormap(map); hold on;
% plot(grid_pts(1,:)+1,grid_pts(2,:)+1,'r+');
% plot(x_box_kk+1,y_box_kk+1,'-b');
% plot(grid_pts(1,ind_corners)+1,grid_pts(2,ind_corners)+1,'mo');
% plot(xorig+1,yorig+1,'*m');
% xlabel('Xc (in camera frame)');
% ylabel('Yc (in camera frame)');
% title('Extracted corners');
% zoom on;
% drawnow;
% hold off;
% pause;

Xi = reshape(([0:n_sq_x]*dX)'*ones(1,n_sq_y+1),Np,1)';
Yi = reshape(ones(n_sq_x+1,1)*[n_sq_y:-1:0]*dY,Np,1)';
Zi = zeros(1,Np);

Xgrid = [Xi;Yi;Zi];


% All the point coordinates (on the image, and in 3D) - for global optimization:

x = grid_pts;
X = Xgrid;


% Saves all the data into variables:

eval(['dX_' num2str(kk) ' = dX;']);
eval(['dY_' num2str(kk) ' = dY;']);  

eval(['wintx_' num2str(kk) ' = wintx;']);
eval(['winty_' num2str(kk) ' = winty;']);

eval(['x_' num2str(kk) ' = x;']);
eval(['sus_' num2str(kk) ' = sus;']);

eval(['X_' num2str(kk) ' = X;']);

eval(['n_sq_x_' num2str(kk) ' = n_sq_x;']);
eval(['n_sq_y_' num2str(kk) ' = n_sq_y;']);

eval(['grid_pts_mat_',num2str(kk),'=grid_pts_mat;']);
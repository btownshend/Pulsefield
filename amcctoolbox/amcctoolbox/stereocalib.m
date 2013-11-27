% function [delta,phi]=stereocalib()
%% Load data

% left
fprintf(1,'Loading the left camera calibration result file Calib_Results_left.mat...\n');

load Calib_Results_left;

fc_left = fc;
cc_left = cc;
kc_left = kc;
alpha_c_left = alpha_c;
    

for kk = 1:n_ima,
   
   if active_images(kk),
       X_left{kk}=eval(['X_',num2str(kk)]);
       omc_left{kk}=eval(['omc_',num2str(kk)]);
       Rc_left{kk}=eval(['Rc_',num2str(kk)]);
       Tc_left{kk}=eval(['Tc_',num2str(kk)]);
       x_left{kk}=Rc_left{kk}*X_left{kk}+repmat(Tc_left{kk},1,size(X_left{kk},2));
   end
end


fprintf(1,'Loading the right camera calibration result file Calib_Results_right.mat...\n');

clear calib_name

load Calib_Results_right;

fc_right = fc;
cc_right = cc;
kc_right = kc;
alpha_c_right = alpha_c;

for kk = 1:n_ima,
   
   if active_images(kk),
       
       X_right{kk}=eval(['X_',num2str(kk)]);
       omc_right{kk}=eval(['omc_',num2str(kk)]);
       Rc_right{kk}=eval(['Rc_',num2str(kk)]);
       Tc_right{kk}=eval(['Tc_',num2str(kk)]);
       x_right{kk}=Rc_right{kk}*X_right{kk}+repmat(Tc_right{kk},1,size(X_right{kk},2));
   end
end


%% Get initial estimate for translation and rotation matrices


om_ref_list = [];
T_ref_list = [];
for ii = 1:n_ima
    if active_images(ii)
        % Align the structure from the first view:
        R_ref = rodrigues(omc_right{ii}) * rodrigues(omc_left{ii})';
        om_ref = rodrigues(R_ref);
        om_ref_list = [om_ref_list om_ref];
    end
end

% initial estimate
om = median(om_ref_list')';



% rotation matrix

load Calib_Results_right.mat;


planes1=GetCameraPlanes2('Calib_Results_left.mat');
planes2=GetCameraPlanes2('Calib_Results_right.mat');

delta0=[0;0;0];
rot0=[0;0;0];

% change to radians
rot0=deg2rad(rot0);
phi0=angvec2dcm(rot0);


options = optimset('LargeScale','on');
options = optimset(options, 'MaxFunEvals', 10000000);
% options = optimset(options, 'MaxIter', 1000);
%Uses the Rodrigues parametrisation

% options = optimset('LevenbergMarquardt','on');
rot0=rodrigues(phi0);
[rot,res] = lsqnonlin(@(rot)phierror(planes1,planes2,rot),rot0,[],[],options);
delta = lsqnonlin(@(delta)deltaerror(planes1,planes2,delta),delta0,[],[],options);
phi=rodrigues(rot);

disp('Initial Estimate');
disp('Delta:');
disp(delta);
disp('Phi:');
disp(rad2deg(rot));


%% Refine estimate

% test
% figure;
% hold on;
% rpts=x_right{1};
% lpts=x_left{1};

% loop until convergence
rpts=0;
lpts=0;
rptspre=rpts+1;
lptspre=lpts+1;

% testing
% delta=[-330;-1;17];

while ~isequal(rpts,rptspre) || ~isequal(lpts,lptspre)
    rptspre=rpts;
    lptspre=lpts;
    rpts=[];
    lpts=[];
    for cntr=1:size(x_left,2);
        [matc,err]=matchpts(x_right{cntr},x_left{cntr},delta,phi);
        lptinds=find(matc);
        rptinds=matc(lptinds);        
        lpts=[lpts,x_left{cntr}(:,lptinds)];
        rpts=[rpts,x_right{cntr}(:,rptinds)];
    end
%     find solution
    deltarot0=[delta;rodrigues(phi)];
    options=optimset('LargeScale','on');
    deltarot=lsqnonlin(@(deltarot)sterroptfn(rpts,lpts,deltarot),deltarot0,[],[],options);
    delta=deltarot(1:3);
    rot=deltarot(4:6);
    phi=rodrigues(rot);
    disp('Delta:');
    disp(delta);
    disp('Phi:');
    disp(rad2deg(rot));
end

% refine solution
deltarot0=[delta;rodrigues(phi)];
options = optimset('LevenbergMarquardt','on');
deltarot=lsqnonlin(@(deltarot)sterroptfn(rpts,lpts,deltarot),deltarot0,[],[],options);
delta=deltarot(1:3);
rot=deltarot(4:6);
phi=rodrigues(rot);
disp('Delta:');
disp(delta);
disp('Phi:');
disp(rad2deg(rot));

figure;
% display results
hold on;
for cntr=1:size(x_left,2);
    [matc,err]=matchpts(x_right{cntr},x_left{cntr},delta,phi,1);
end

% display matching points
% lptst=phi*lpts+repmat(delta,1,size(lpts,2));
% scatter3(rpts(1,:),rpts(2,:),rpts(3,:),[],1:size(rpts,2),'+');
% scatter3(lptst(1,:),lptst(2,:),lptst(3,:),[],matc,'+');


% use initial estimate to match points


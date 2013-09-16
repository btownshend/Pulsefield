% Layout camera positions on floor
function layout=layoutfloor
camorder=[3,4,5,1,2,6];
%camorder=1:6;
% Measurements 9/15/13 +/- 1 inch
cdist=[0 77 141 275 284.5 282
       0  0 67.5 238 265.5 284.5
       0  0  0  196.5 239 275
       0  0  0   0  70 143
       0  0  0   0   0  76.5
       0 0 0 0 0 0];
cdist=(cdist+cdist');  % Make symmetric
cdist(camorder,camorder)=cdist;   % Reorder

% MD scaling to convert distances into locations
[y,e]=cmdscale(cdist);
y(:,2)=-y(:,2);   % Flip it over

% Adjust based on knowing they are on a circle
origin=mean(y(camorder([1,2,5,6]),1:2));   % Asymmetric
for i=1:size(y,1)
  y(i,1:2)=y(i,1:2)-origin;
end
radius=mean(sqrt(y(:,1).^2+y(:,2).^2))/39.37;

% Compute error in measurements
adist=zeros(size(cdist));
err=[];
for i=1:size(y,1)
  for j=i+1:size(y,1)
    adist(i,j)=norm(y(i,:)-y(j,:));
    err(end+1)=norm(adist(i,j)-cdist(i,j));
  end
end
fprintf('RMS error in distances = %.1f inches\n', sqrt(mean(err.^2)));

% Build layout structure
cpos=y(:,1:2)/39.37;
cdir=zeros(size(cpos));
for i=1:size(cpos,1)
  dir=-cpos(i,:);
  cdir(i,:)=dir/norm(dir);
end
pos=[];
[pos(:,1),pos(:,2)]=pol2cart(0:pi/20:2*pi,radius);
layout=struct('cpos',cpos,'cdir',cdir,'lpos',nan(640,2),'ldir',nan(640,2),'outsider',false(640,1),'active',pos,'entry',mean(cpos([3,4],:)),'pos',pos);



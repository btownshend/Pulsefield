% Test whether we can extract individual projector poses from correspondences

for i=1:2
  p{i}=rand(3,3);   % Mapping matrix for each projector from 3d space (with z=0) to projector space
  p{i}(3,3)=1;
end


% Choose some random points on plane
N=10;
pts=[rand(N,1)*10-5,rand(N,1)*5];
pts(:,3)=1;   % Homogenous

setfig('simulate');clf;
subplot(221);
plot(pts(:,1),pts(:,2),'o');
axis equal;
title('World');
anchors={};
for i=1:2
  subplot(2,2,i+2);
  pp=p{i}*pts';
  pp=pp';
  pp(:,1)=pp(:,1)./pp(:,3);
  pp(:,2)=pp(:,2)./pp(:,3);
  plot(pp(:,1),pp(:,2),'o');
  axis equal;
  title(sprintf('Projector %d',i));
  anchors{i}=pp(:,1:2);
end


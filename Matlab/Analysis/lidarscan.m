% Compute LIDAR scan times for points along a line in the middle
[x,y]=meshgrid(-15:15,-15:15);
t=[];
for i=1:size(x,1)
  for j=1:size(x,2)
    t(i,j,:)=scantime([x(i,j),y(i,j)]);
  end
end
setfig('lidarscan');clf
tdiff=abs(t(:,:,1)-t(:,:,2));
tdiff=min(tdiff,.020-tdiff);
surf(x,y,tdiff*1000);
shading interp
colorbar
xlabel('X Pos (ft)');
ylabel('Y Pos (ft)');
zlabel('Time Diff (ms)');

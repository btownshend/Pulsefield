function refinestereocalib(delta,phi)

BoardCorners1=GetBoardCorners2('Calib_Results_right.mat');
BoardCorners2=GetBoardCorners2('Calib_Results_left.mat');

pts1=BoardCorners1(1).corners;
pts2=BoardCorners2(1).corners;
pts2=phi*pts2+repmat(delta,1,size(pts2,2));

figure;
hold on;
plot3(pts1(1,:),pts1(2,:),pts1(3,:),'+');
plot3(pts2(1,:),pts2(2,:),pts2(3,:),'r+');

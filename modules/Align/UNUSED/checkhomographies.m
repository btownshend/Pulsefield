% Data from calibrator.cc output
hfwd={};  % Maps from projector to world
hinv={};  % Maps from world coordinates to projector
pose={};  
d=[-0.000513 -0.005307 2.581812 -0.004181 0.003195 5.912557 -0.000156 0.000672 1.000000];
hfwd{end+1}=reshape(d,3,3)';
d=[32.753078 -295.589172 1663.125488 -136.653717 4.584259 325.709473 0.097011 -0.049326 1.000000];
hinv{end+1}=reshape(d,3,3)';
d=[ 0.004480 0.002009 -5.249223 0.001683 -0.004406 4.910583 0.000135 0.000739 1.000000];
hfwd{end+1}=reshape(d,3,3)';
d=[347.447083 254.594009 573.622253 44.002354 -224.554016 1333.669189 -0.079573 0.131365 1.000000];
hinv{end+1}=reshape(d,3,3)';
d=[1.000000 0.000000 0.000000 0.000000 1.000000 0.000000 0.000000 0.000000 1.00000];
hfwd{end+1}=reshape(d,3,3)';
d=[ 1.000000 -0.000000 0.000000 0.000000 1.000000 -0.000000 0.000000 -0.000000 1.000000];
hinv{end+1}=reshape(d,3,3)';
d=[0.233078 -0.999880 0.000152 7.626413 -0.972458 0.015507 -0.000651 1.493570 0.000690 -0.000167 -0.968727 0.004586];
pose{end+1}=reshape(d,4,3)';
d=[0.992076 0.749966 -0.000102 1.663405 0.125641 -0.661476 -0.000554 3.867409 -0.000227 0.000387 -0.750461 0.002900];
pose{end+1}=reshape(d,4,3)';

% Test mappings
setfig('homog');clf;
for k=1:2
  fprintf('\nProjector %d\n', k);
  world=[0 0
         0 1
         1 0
         2 2];
  for i=1:size(world,1)
    proj=hinv{k}*[world(i,:),1]';
    proj=proj(1:2)/proj(3);
    fprintf('World [%.2f,%.2f] -> Proj [%.0f %.0f]\n', world(i,:), proj);
  end
  proj=[0 0
        1920 0
        1920 1080
        0 1080
        0 0
        1920/2 1080/2];
  w=[];
  for i=1:size(proj,1)
    world=hfwd{k}*[proj(i,:),1]';
    world=world(1:2)/world(3);
    fprintf('Proj [%.0f,%.0f] -> World [%.2f %.2f]\n', proj(i,:), world);
    w(i,:)=world;
  end
  plot(w(:,1),w(:,2));
  hold on;
end
p1_to_p2=hfwd{1}*hinv{2};
p1_to_p2=p1_to_p2/p1_to_p2(3,3);
p2_to_p1=hfwd{2}*hinv{1};
p2_to_p1=p2_to_p1/p2_to_p1(3,3);



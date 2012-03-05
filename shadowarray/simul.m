% Simulate detection of blockage of a line array
setup

spos=calcspos(cpos,cdir,lpos,p);

settargets

% Draw layout
figure(1);
clf;
hold on;
axis ij
axis equal
axis off
% Find visibility of each LED
v=[];
v=calcvisible(cpos,cdir,p.cam.fov,lpos,tpos,tgtdiam);

% Plot layout
plot(lpos(:,1),lpos(:,2),'.');
plot(cpos(:,1),cpos(:,2),'og');
viscircles(tpos,tgtdiam/2,'LineWidth',0.5);
fprintf('Percent of LEDs visible = %.1f%%\n', 100*sum(v(:)==1)/sum(isfinite(v(:))));
% Plot FOV
for i=1:size(cpos,1)
  ca(1)=atan2(cdir(i,2),cdir(i,1))+p.cam.fov/2;
  ca(2)=atan2(cdir(i,2),cdir(i,1))-p.cam.fov/2;
  for j=1:2
    endpos=cpos(i,:)+(p.cextend+p.sidelength/2)*[cos(ca(j)),sin(ca(j))];
    plot([cpos(i,1),endpos(1)],[cpos(i,2),endpos(2)],'k');
  end
end

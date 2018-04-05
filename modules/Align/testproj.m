% Test projection mapping
proj=1;
ptest=rand(5000,2);
tic
[cx,cy]=proj2cam(pmap{proj},ptest(:,1),ptest(:,2));
toc
tic
[px,py]=cam2proj(pmap{proj},cx,cy);
toc
ctest=rand(1000,2);
ctest(:,1)=ctest(:,1)*1824;
ctest(:,2)=ctest(:,2)*1376;
[cpx,cpy]=cam2proj(pmap{proj},ctest(:,1),ctest(:,2));
[cx2,cy2]=proj2cam(pmap{proj},cpx,cpy);

for i=1:min(20,size(ptest,1))
  fprintf('[%.2f,%.2f] -> [%.0f,%.0f] -> [%.2f,%.2f]\n', ptest(i,:),cx(i),cy(i),px(i),py(i));
end

setfig('testproj');clf;
subplot(211);
plot(ptest(:,1),ptest(:,2),'.');
hold on;
plot(px,py,'.');
xlabel('Proj (x)');
ylabel('Proj (y)');
axis equal;
axis([0,1,0,1]);
subplot(212);
plot(ctest(:,1),ctest(:,2),'.');
hold on;
plot(cx2,cy2,'.');
xlabel('Cam (x)');
ylabel('Cam (y)');
axis equal;
axis([0,1823,0,1375]);
  
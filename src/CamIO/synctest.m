% Test sync between multiple arecont cameras
% Start LED chaser running first
ids=[1,2];
nled=160-60;
dt=.0163;
tl=[1400,1950
    1450,1100];
roi={[tl(1,1),tl(1,1)+1300,tl(1,2),tl(1,2)+150],
     [tl(2,1),tl(2,1)+1300,tl(2,2),tl(2,2)+150]}
for i=1:length(ids)
  id=ids(i);
  arecont_set(id,'autoexp','on');
  arecont_set(id,'exposure','on');
  arecont_set(id,'brightness',-50);
  arecont_set(id,'lowlight','highspeed');
  arecont_set(id,'shortexposures',10);
  arecont_set(id,'maxdigitalgain',32);
  arecont_set(id,'analoggain',1);
%  pause(2);
%  arecont_set(id,'autoexp','off');
%  arecont_set(id,'exposure','off');
end

figure(1);
clf;
mv=[];
for j=1:50
  im=aremulti(ids,roi);
  for i=1:length(ids)
    subplot(length(ids),1,i);
    [~,mpos]=max(max(max(im2double(im{i}),[],3),[],1));
    mv(j,i)=mpos;
    if j==1
      imshow(im{i});
    end
  end
end
figure(2);
plot(mv(:,1),mv(:,2),'.');
xlabel(sprintf('Camera %d',ids(1)));
ylabel(sprintf('Camera %d',ids(2)));
figure(3);
mvs=mv;
for i=1:2
  mvs(:,i)=mvs(:,i)-min(mvs(:,i));
  mvs(:,i)=mvs(:,i)/max(mvs(:,i))*nled*dt*1000;
end
plot(mvs(:,2)-mvs(:,1),'.');
xlabel('Sample');
ylabel('2-1 (msec)');
%axis([1 size(mvs,1) -0.1 0.1]);
offset=median(abs(mvs(:,2)-mvs(:,1)));
fprintf('Median lag is %.1f msec.\n', offset);
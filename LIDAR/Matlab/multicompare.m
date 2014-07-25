% Compare the results of one multi run to another
m={multi1,multi2};
p=[];
for mi=1:length(m)
  for i=1:length(m{mi})
    nid=length(m{mi}(i).tracker.tracks);
    for j=1:nid
      p(mi,i,j,:)=m{mi}(i).tracker.tracks(j).position;
    end
  end
end

e=[];
for i=1:size(p,3)
  for j=1:size(p,3)
    diff=squeeze(p(1,:,i,:)-p(2,:,j,:));
    e(i,j)=sum(diff(:,1).^2+diff(:,2).^2);
    fprintf('e(%d,%d)=%f\n', i,j,e(i,j));
  end
end
[~,matched]=min(e(:));
[i,j]=ind2sub(size(e),matched);
p1=squeeze(p(1,:,i,:));
p2=squeeze(p(2,:,j,:));
setfig('multicompare'); clf;
subplot(3,1,1);
plot(p1(:,1),'.r');
hold on;
plot(p2(:,1),'.g');
xlabel('frame');
ylabel('x-position');
subplot(3,1,2);
plot(p1(:,2),'.r');
hold on;
plot(p2(:,2),'.g');
xlabel('frame');
ylabel('y-position');
subplot(3,1,3);
diff=p1-p2;
plot(sqrt(diff(:,1).^2+diff(:,2).^2),'.');
xlabel('frame');
ylabel('delta');

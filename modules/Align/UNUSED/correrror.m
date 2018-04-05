function e=correrror(corr,x)
c1=corr(:,[1,2]);
c1(:,3)=1;
c2=corr(:,[3,4]);
x1=reshape(x,3,3);
x1(3,3)=1;
c1x=c1*x1;
c1x(:,1)=c1x(:,1)./c1x(:,3);
c1x(:,2)=c1x(:,2)./c1x(:,3);
err=c2(:,1:2)-c1x(:,1:2);
e=mean(err(:).^2);


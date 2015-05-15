% Quick test of finding corner using regressions
pts1 = [-0.476446,10.2305
        -0.526658,10.0502];
pts2 = [1.89974,1.26564
        1.89419,1.27792
        1.88784,1.28968
        1.92024,1.32826];
pts=[pts1;pts2];
fit1=orthfit(pts1(:,1),pts1(:,2));
fit2=orthfit(pts2(:,1),pts2(:,2));
x=min(pts(:,1)):(max(pts(:,1))-min(pts(:,1)))/100:max(pts(:,1));
setfig('regress');clf;
plot(pts(:,1),pts(:,2),'x');
hold on;
plot(x,polyval(fit1,x),'r');
plot(x,polyval(fit2,x),'g');
xc=(fit1(2)-fit2(2))/(fit2(1)-fit1(1));
yc=polyval(fit1,xc);
plot(xc,yc,'o');
fprintf('fit1 = %f*x + %f\n', fit1);
fprintf('fit2 = %f*x + %f\n', fit2);

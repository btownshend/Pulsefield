% What does a triangle look like from the edge
th=0:0.01:2*pi;
for i=1:length(th)
  theta=th(i);
  x(1)=cos(theta);
  x(2)=cos(theta+2*pi/3);
  x(3)=cos(theta+2*pi*2/3);
  x=sort(x);
  r(i)=(x(2)-x(1))/(x(3)-x(1));
end
plot(th*180/pi,r);
xlabel('Theta');
ylabel('Ratio');
  
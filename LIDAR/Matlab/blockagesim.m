% Simulate blockage by people
active=9;  % width=height of active area (in meters)
border=active*0.1276;   % Minimum distance from LIDAR to active region (in meters) (based on Optoma image offset)
tgtsize=0.27;  % average cross-section of each target 
area=active^2;
% Check all positions relative to LIDAR at (0,0)
angle=[];   % Angle of person
coverage=[];% Angle of view occupied by person
for x=-active/2:0.01:active/2
  for y=border:0.01:active+border
    d=sqrt(x^2+y^2);
    coverage(end+1)=tgtsize/d;
    angle(end+1)=atan2(y,x);
  end
end
qstep=(max(angle)-min(angle))/100;
qangle=min(angle)+qstep/2:qstep:max(angle)-qstep/2;
qcoverage=[];
qmin=[];qmax=[];
prob=[];
for i=1:length(qangle)
  qcoverage(i)=mean(coverage(angle>=qangle(i)-qstep/2 & angle<=qangle(i)+qstep/2));
  qmin(i)=min(coverage(angle>=qangle(i)-qstep/2 & angle<=qangle(i)+qstep/2));
  qmax(i)=max(coverage(angle>=qangle(i)-qstep/2 & angle<=qangle(i)+qstep/2));
  prob(i)=mean(angle>=qangle(i)-qstep/2 & angle<=qangle(i)+qstep/2);
end
setfig('blockages');clf;
subplot(211);
plot(qangle*180/pi,qcoverage*180/pi);
hold on;
plot(qangle*180/pi,qmin*180/pi,':');
plot(qangle*180/pi,qmax*180/pi,':');
xlabel('View angle (deg)');
ylabel('Field blocked (deg)');
c=axis;c(3)=0;axis(c);
yyaxis right
plot(qangle*180/pi,prob);
ylabel('Prob');
meancover=sum(qcoverage.*prob)/sum(prob);
fov=(qangle(end)-qangle(1));
fprintf('Each person blocks an average of %.2f degrees over a total field of %.2f degrees\n', meancover*180/pi, fov*180/pi);
subplot(212);
npeople=0:50;
vis1=(1-meancover/fov).^npeople;
plot(npeople,vis1);
axis([0,max(npeople),0,1])
hold on;
vis2=1-(1-vis1).^2;
plot(npeople,vis2);
legend('1 scanner','2 scanners','Location','SouthWest');
xlabel('Num people');
ylabel('Fraction visible');
minvis=0.9;
plot([0,max(npeople)],minvis*[1,1],':k');
n1=log(minvis)/log(1-meancover/fov)
fprintf('For %.0f%% visibility, max people=%d, %d\n', minvis*100, max(npeople(vis1>=minvis)),max(npeople(vis2>=minvis)));


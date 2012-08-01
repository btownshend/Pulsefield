% Test speed of getvisible
ntest=50;
% Turn on LEDs
s1=arduino_ip(1);
setled(s1,[0,numled()-1],127*p.colors{1},1); show(s1); sync(s1);
pause(2);
vis={};
fprintf('Running getvisible() %d times\n', ntest);
cpustart=cputime;
tic
profile on
for i=1:ntest
  vis{i}=getvisible(p,'setleds',false);
end
profile off
dt=toc;
cpuelapsed=cputime-cpustart;
setled(s1,-1,[0,0,0],1);show(s1);
fprintf('Mean rate = %.1f FPS, Processor=%.0f%%\n', ntest/dt,100*cpuelapsed/dt);
% Check results
for c=1:size(vis{i}.v,1)
  for i=1:ntest
    noff(i,c)=sum(vis{i}.v(c,:)==0);
  end
  fprintf('Camera%d:  Num LEDs blocked = %d - %d\n', c, min(noff(:,c)),max(noff(:,c)));
end
plotvisible(p,vis{end});
profile viewer

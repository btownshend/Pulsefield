s1=arduino_ip(1);
sync(s1);
nsync=5;
nrpt=ceil(400/nsync);
fprintf('nsync=%d, nrpt=%d\n', nsync, nrpt);
while true
  tic;
  for rpt=1:nrpt
    for i=1:nsync
      cntr(i)=syncsend(s1,1);
    end
    for i=1:nsync
      syncwait(s1,cntr(i));
    end
  end
  elapsed=toc;
  fprintf('%.0f syncs/sec\n',nsync*nrpt/elapsed);
  pause(rand);
end
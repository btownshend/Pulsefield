% Run loopback test
function ok=oscloopback(myident)
  % Loopback test to make sure ports are right
  seed=randi(10000);
  while true
    m=oscmsgin(myident,0.0);   % Flush and make sure connection is open
    if isempty(m)
      break;
    end
  end
  oscmsgout(myident,'/loopback',{int32(seed)},true);
  while true
    m=oscmsgin(myident,1.0);
    if isempty(m)
      [~,hostname]=system('hostname');
      fprintf('Failed loopback test to %s@%s while running on %s',myident,getsubsysaddr(myident),hostname);
      ok=false;
      return;
    end
    if strcmp(m.path,'/loopback') && m.data{1}==seed
      fprintf('Loopback test passed!\n');
      break;
    end
    fprintf('Loopback test: flushing %s\n', formatmsg(m.path,m.data));
  end
  ok=true;
end
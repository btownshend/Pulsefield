% Snapshot server
% Receive commands via OSC, take regular snapshots when PF is occupied
function snapserver
  pfile=[pfroot(),'/Calibration/current.mat'];
  p=load(pfile);
  
  debug=0;
  ignores={};

  if ~oscloopback('SN')
    fprintf('Loopback failed - exiting Snapserver()\n');
    oscclose();
    return;
  end
  oscclose();   % Clean up any old connections

  [~,myport]=getsubsysaddr('SN');
  fprintf('Instructing Matlab processor to use port %d to send us msgs\n', myport);
  oscmsgout('MPO','/pf/dest/add/port',{myport,'SN'},debug);
  oscmsgout('TO','/snapshot/label/color',{'green'});

  period=30;
  lastsnap=0;
  npeople=0;
  trigger=false;
  
  while true
    % Receive any OSC messages
    if npeople>0
      maxwait=max(0.0,(lastsnap-now)*24*3600+period);
    else
      maxwait=1.0;
    end
    m=oscmsgin('SN',maxwait);
    if ~isempty(m)
      fprintf('Got message: %s\n', formatmsg(m.path,m.data));
      if strcmp(m.path,'/pf/set/npeople')
        if npeople==0 && m.data{1}>0
          fprintf('Someone entered - starting snapshots\n');
        elseif npeople>0 && m.data{1}==0
          fprintf('Last exit - stopping snapshots\n');
        end
        npeople=m.data{1};
      elseif strcmp(m.path,'/snapshot/trigger') && m.data{1}==1
        trigger=true;
      else
        if ~ismember(m.path,ignores)
          fprintf('Snapserver: Unhandled OSC message from %s: %s - ignoring from now on.\n', m.src, m.path);
          ignores{end+1}=m.path;
        end
      end
    end
    if trigger || ((now-lastsnap)*3600*24>=period  && npeople>0)
      oscmsgout('TO','/snapshot',{int32(1)});
      snapshot(p,true);
      oscmsgout('TO','/snapshot',{int32(0)});
      lastsnap=now;
      trigger=false;
    end
  end
end
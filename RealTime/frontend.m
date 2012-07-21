% Front end processing
% Acquires images from cameras, reduces to vis struct
% Sends results to named pipe where second stage picks up processing
function frontend
setupfile='/tmp/pulsefield_setup.mat';
recvis=load(setupfile);
pipefile='/tmp/pulsefield.pipe';
pipeversion=1;
if ~exist(pipefile,'file')
  % Doesn't exist
  cmd=sprintf('mkfifo %s',pipefile);
  fprintf('Creating pipe: %s...',cmd);
  [s,r]=system(cmd);
  if s~=0
    error('\nError running "%s": %s\n', cmd, r);
  end
  fprintf('done\n');
end
pipefd=fopen(pipefile,'wb');
fwrite(pipefd,sprintf('VISD-VER%04d',pipeversion),'char*1');
fwrite(pipefd,recvis.randseed,'double');   % Seed to verify that save file matches stream
fwrite(pipefd,length(recvis.p.camera),'uint16');
fwrite(pipefd,length(recvis.p.led),'uint16');
frame=1;
while true
  fstart=now;
  fprintf('Frame %d: Acquiring...', frame);
  vis=getvisible(recvis.p,'setleds',false);
  fprintf('done in %.2f sec.  ',(now-fstart)*24*3600);
  fstart=now;
  binary=uint8(vis.v);
  binary(isnan(vis.v))=2;
  fprintf('Sending...');
  try
    c=fwrite(pipefd,'FRME','char*1');
    fwrite(pipefd,frame,'uint32');
    fwrite(pipefd,vis.when,'double');
    cnt=fwrite(pipefd,binary,'uint8');
    if cnt~=length(binary(:))
      error('Only wrote %d/%d entries to pipe\n', cnt, length(binary(:)));
    end
  catch me
    error('Error sending to pipe: %s\n', me.message);
  end
  fprintf('done in %.2f sec\n',(now-fstart)*24*3600);
  frame=frame+1;
end

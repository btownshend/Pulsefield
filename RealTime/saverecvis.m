recvis.note=input('Enter comment for save file (or return to skip save): ','s');
if ~isempty(recvis.note)
  recname=sprintf('Recordings/%s.mat',datestr(recvis.snap(1).when,30));
  fprintf('Saving recording in %s...', recname);
  if isfield(recvis.vis,'im')
    recvistmp=rmfield(recvis,'vis');
    recvistmp.vis=rmfield(recvis.vis,'im');
    save(recname,'-struct','recvistmp');
  else
    save(recname,'-v7.3','-struct','recvis');
  end
  fprintf('done\n');
end

recvis.note=input('Enter comment for save file (or return to skip save): ','s');
if ~isempty(recvis.note)
  recname=sprintf('Recordings/%s.mat',datestr(vis.snap{1}.when,30));
  fprintf('Saving recording in %s...', recname);
  recvistmp=rmfield(recvis,'vis');
  if isfield(recvis.vis,'im')
    recvistmp.vis=rmfield(recvis.vis,'im');
  end
  save(recname,'-struct','recvistmp');
  fprintf('done\n');
end

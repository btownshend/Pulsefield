recvis.note=input('Enter comment for save file (or return to skip save): ','s');
if ~isempty(recvis.note)
  recname=sprintf('Recordings/%s.mat',datestr(vis.when,30));
  fprintf('Saving recording in %s...', recname);
  recvistmp=rmfield(recvis,'vis');
  recvistmp.vis=rmfield(recvis.vis,'im');
  save(recname,'-struct','recvistmp');
  fprintf('done\n');
end

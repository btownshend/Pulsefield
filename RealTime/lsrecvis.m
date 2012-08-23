% List recordings
files=dir('Recordings/*.mat');
for i=1:length(files)
  f=files(i);
  zz=load(['Recordings/',f.name],'note');
  if ~isfield(zz,'note')
    system(sprintf('mv Recordings/%s Recordings/BAD',f.name));
    fprintf('Moved %s to BAD\n', f.name);
  end
  if ~isfield(zz,'note')
    zz.note='(empty)';
  end
  if ~isfield(zz,'vis')
    zz.vis=[];
  end
  fprintf('%s %s %4.1fMB %s\n', f.name, f.date, round(f.bytes/1024/1024), zz.note);
end
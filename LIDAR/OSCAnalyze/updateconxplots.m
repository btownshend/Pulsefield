% update all plots of connections
basedir='../Recordings/Conductor';
files=dir(basedir);
for i=1:length(files)
  if length(files(i).name)>=5 && strcmp(files(i).name(end-3:end),'.osc')
    fullname=[basedir,'/',files(i).name];
    tifname=strrep(fullname,'.osc','.tif');
    if ~exist(tifname,'file')
      fprintf('Updating %s -> %s\n', fullname, tifname);
      plotconxs(fullname);
      print('-dtiff','-r300',tifname);
    end
  else
    fprintf('Skipping %s\n', files(i).name);
  end
end

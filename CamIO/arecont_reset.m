function arecont_reset(id)
[h,p]=getsubsysaddr(sprintf('CA%d',id));
url=sprintf('http://%s/set?params=factory',h);
cmd=sprintf('curl -s ''%s'' &', url);
system(cmd);

function arecont_reset(id)
url=sprintf('http://192.168.0.%d/set?params=factory',id+70);
cmd=sprintf('curl -s ''%s'' &', url);
system(cmd);

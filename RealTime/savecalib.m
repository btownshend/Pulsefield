% Save calibration
function savecalib(p)
dirname='Calibration';
filename=sprintf('calib-%s.mat',datestr(now,30));
fullname=[dirname,'/',filename];
fprintf('Saving calibration in %s...',fullname);
save(fullname,'-struct','p');
fprintf('linking to %s/current.mat...',dirname);
cmd=sprintf('rm -f "%s/current.mat"; ln -s "%s" "%s/current.mat"',dirname,filename,dirname);
system(cmd);
fprintf('done\n');
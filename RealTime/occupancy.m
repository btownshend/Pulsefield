function occupancy
global recvis
t=(arrayfun(@(z) z.when, recvis.snap)-recvis.snap(1).when)*24*3600;
occ=arrayfun(@(z) length(z.hypo), recvis.snap);
plot(t,occ);
xlabel('Time (sec)');
ylabel('Occupancy');
title(sprintf('%s (%.1f fps)', recvis.note,length(recvis.snap)/t(end)));


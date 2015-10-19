% Map from a fractional power of maximum (in watts) to setting needed in percent
function setting=mappower(frac)
nominal=[100,90,85,76,69,66,60,56,50,46,44,41,39,37,35,32]/100;
watts=[15.38,14.69,14.52,13.60,12.26,11.00,9.72,7.81,4.22,3.11,2.32,1.41,0.94,0.25,0.05,0];
actual=watts/watts(1);
setting=interp1(actual,nominal,frac);

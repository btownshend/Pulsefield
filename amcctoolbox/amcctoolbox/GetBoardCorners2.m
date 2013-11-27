function BoardCorners=GetBoardCorners2(filename)

load(filename);

stringRBase = 'Rc_';
stringTBase = 'Tc_';
stringXBase = 'X_';
stringnsqxBase='n_sq_x_';
stringnsqyBase='n_sq_y_';

kk=1;
while exist([stringXBase,num2str(kk)])
    rc = eval([stringRBase,num2str(kk)]);
    tc = eval([stringTBase,num2str(kk)]);
    x  = eval([stringXBase,num2str(kk)]);
    BoardCorners(kk).n_sq_x = eval([stringnsqxBase,num2str(kk)]);
    BoardCorners(kk).n_sq_y = eval([stringnsqyBase, num2str(kk)]);
    BoardCorners(kk).corners=(rc * x + tc * ones(1,size(x,2)))./1000; % in m
    kk=kk+1;
end
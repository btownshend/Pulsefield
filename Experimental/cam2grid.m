% Convert from Camera coordinate space to Grid (chessboard) coordinate space
function g=cam2grid(r,c)
g=inv(r.Rc)*(c-r.Tc);

% Convert from Camera coordinate space to Grid (chessboard) coordinate space
function c=grid2cam(r,g)
c=r.Rc*g+r.Tc;

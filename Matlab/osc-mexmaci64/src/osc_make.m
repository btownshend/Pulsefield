function osc_make()
cd(fileparts(which('osc_make')))
mex -O -v -L/opt/local/lib -I../../liblo-0.26 -llo osc_new_address.c
mex -O -v -L/opt/local/lib -I../../liblo-0.26 -llo osc_free_address.c
mex -O -v -L/opt/local/lib -I../../liblo-0.26 -llo osc_new_server.c
mex -O -v -L/opt/local/lib -I../../liblo-0.26 -llo osc_free_server.c
mex -O -v -L/opt/local/lib -I../../liblo-0.26 -llo osc_send.c
mex -O -v -L/opt/local/lib -I../../liblo-0.26 -llo osc_recv.c

SRCS=OSCListener.ck globals.ck generator.ck stkgenerator.ck fmgenerator.ck main.ck
main:
	chuck --verbose:3 $(SRCS)

dump:
	chuck --dump $(SRCS)

test:
	chuck OSCListener.ck person.ck pulsefield.ck test.ck

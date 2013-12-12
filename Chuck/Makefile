SRCS=OSCListener.ck globals.ck BPM.ck pattern.ck generator.ck stkgenerator.ck fmgenerator.ck main.ck record.ck
VERBOSE=1

main:
	chuck --verbose:$(VERBOSE) $(SRCS)

dump:
	chuck --dump $(SRCS)

test:
	chuck OSCListener.ck person.ck pulsefield.ck test.ck

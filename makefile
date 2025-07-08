.PHONY: all sub1 sub2 sub3

all: sub1 sub2 sub3

sub1:
	$(MAKE) -C ./common/scintilla_370/

sub2:
	$(MAKE) -C ./approbot

sub3:
	$(MAKE) -C ./appcad


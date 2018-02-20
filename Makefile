SUBDIRS := pli2asm asm2obj ibm360vm

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)

clear:
	rm -f build/*
SUBDIRS=lib untar stdin

.PHONY:		subdirs $(SUBDIRS) clean

subdirs:	$(SUBDIRS)

$(SUBDIRS):	
		$(MAKE) -C $@
		
clean:		
		for dir in $(SUBDIRS); do \
			$(MAKE) -C $$dir clean; \
		done

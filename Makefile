BUILD_SUBDIRS = threads userprog vm filesys

all::
	@echo "Running 'make' in subdirectories: $(BUILD_SUBDIRS)."
	@for dir in $(BUILD_SUBDIRS) utils; do $(MAKE) -C $$dir; done

CLEAN_SUBDIRS = $(BUILD_SUBDIRS) examples utils

clean::
	for d in $(CLEAN_SUBDIRS); do $(MAKE) -C $$d $@; done
	rm -f TAGS tags

distclean:: clean
	find . -name '*~' -exec rm '{}' \;

TAGS_SUBDIRS = $(BUILD_SUBDIRS) devices lib
TAGS_SOURCES = find $(TAGS_SUBDIRS) -name \*.[chS] -print

TAGS::
	etags --members `$(TAGS_SOURCES)`

tags::
	ctags -T --no-warn `$(TAGS_SOURCES)`

cscope.files::
	$(TAGS_SOURCES) > cscope.files

cscope:: cscope.files
	cscope -b -q -k